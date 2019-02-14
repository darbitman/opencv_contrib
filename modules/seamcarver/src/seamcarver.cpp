#include "opencv2/seamcarver/seamcarver.hpp"
#include <chrono>
using namespace std::chrono;
#include <iostream>
#ifdef USEDEBUGDISPLAY
#include "DebugDisplay.h"
#endif

bool cv::SeamCarver::findAndRemoveVerticalSeams(int32_t numSeams, const cv::Mat& img,
                                                 cv::Mat& outImg, cv::energyFunc computeEnergyFn)
{
    this->numRows_ = img.rows;
    this->numColumns_ = img.cols;
    this->bottomRow_ = numRows_ - 1;
    this->rightColumn_ = numColumns_ - 1;
    this->posInf_ = std::numeric_limits<double>::max();

    // check if removing more seams than columns available
    if (numSeams > numColumns_)
    {
        return false;
    }

    /*** DECLARE VECTORS THAT WILL BE USED THROUGHOUT THE SEAM REMOVAL PROCESS ***/
    // output of the function to compute energy
    // input to the CurrentSeam finding function
    vector<vector<double>> PixelEnergy;
    // resize output if necessary
    PixelEnergy.resize(numRows_);
    for (int r = 0; r < numRows_; r++)
    {
        PixelEnergy[r].resize(numColumns_);
    }

    // output of the CurrentSeam finding function
    // input to the CurrentSeam removal function
    // vector of minimum-oriented priority queues. Each row in the vector corresponds to a
    //      priority queue for that row in the image
    vectorOfMinPQ seams;
    // TODO fix warning C4239 due to the resize function
    seams.resize(numRows_);

    // make sure markedPixels hasn't been set before
    // resize markedPixels matrix to the same size as img;
    if (markedPixels.size() != (uint32_t)numRows_)
    {
        markedPixels.resize(numRows_);
        for (int32_t r = 0; r < numRows_; r++)
        {
            markedPixels[r].resize(numColumns_);
            for (int32_t c = 0; c < numColumns_; c++)
            {
                markedPixels[r][c] = false;
            }
        }
    }

    // vector to store the image's channels separately
    vector<cv::Mat> bgr;
    bgr.resize(3);
    cv::split(img, bgr);

    try
    {
        // allocate min-oriented priority queue for each row to hold numSeams elements
        for (int32_t r = 0; r < numRows_; r++)
        {
            // TODO uncomment this once ConstSizeMinBinaryHeap is fixed/incorporated
            //if (!seams[r].allocate(numSeams))
            //{
            //    throw std::exception("Could not allocate memory for min oriented priority queue");
            //}
        }

        auto start = high_resolution_clock::now();
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);

        // Compute pixel energy
        if (computeEnergyFn == nullptr)
        {
            start = high_resolution_clock::now();
            pixelEnergyCalculator_.setDimensions(numColumns_, numRows_, img.channels());
            pixelEnergyCalculator_.calculatePixelEnergy(img, PixelEnergy);
            stop = high_resolution_clock::now();
            duration = duration_cast<microseconds>(stop - start);

#ifdef USEDEBUGDISPLAY
            KDebugDisplay d;
            d.Display2DVector<double>(PixelEnergy, PixelEnergyCalculator_.GetMarginEnergy());
#endif
        }
        else
        {
            // TODO refactor names/parameters associated with user defined function
            // call user-defined energy computation function
            computeEnergyFn(img, PixelEnergy);
        }

        // find all vertical seams
        start = high_resolution_clock::now();
        this->findVerticalSeams(numSeams, PixelEnergy, seams); // ~2.5s
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);

        // remove all found seams
        start = high_resolution_clock::now();
        //this->markVerticalSeams(bgr, seams);
        this->removeVerticalSeams(bgr, seams);  // ~55ms
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);

        // combine separate channels into output image
        start = high_resolution_clock::now();
        cv::merge(bgr, outImg); // ~300-400us
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
        //this->markVerticalSeams(bgr, seams);
        //this->markInfEnergy(bgr, pixelEnergy);
        //cv::merge(bgr, outImg);
        return false;
    }

    return true;
}


bool cv::SeamCarver::findVerticalSeams(int32_t numSeams, vector<vector<double>>& pixelEnergy,
                                        vectorOfMinPQ& outDiscoveredSeams)
{
    if (pixelEnergy.size() == 0)
    {
        throw std::out_of_range("Pixel energy vector is empty\n");
    }

    if (outDiscoveredSeams.size() != pixelEnergy.size())
    {
        throw std::out_of_range("OutDiscoveredSeams does not have enough rows\n");
    }

    int32_t SeamRecalculationCount = 0;

    // TotalEnergyTo will store cumulative energy to each pixel
    // ColumnTo will store the columnn of the pixel in the row above to get to current pixel
    vector<vector<double>> TotalEnergyTo;
    vector<vector<int32_t>> ColumnTo;

    // resize number of rows
    TotalEnergyTo.resize(numRows_);
    ColumnTo.resize(numRows_);

    // resize number of columns for each row
    for (int32_t r = 0; r < numRows_; r++)
    {
        TotalEnergyTo[r].resize(numColumns_);
        ColumnTo[r].resize(numColumns_);
    }

    // initial path calculation
    this->calculateCumulativeVerticalPathEnergy(pixelEnergy, TotalEnergyTo, ColumnTo);

    // temporary CurrentSeam to verify that there are no previously markedPixels
    //      in this CurrentSeam
    // otherwise the cumulative energies need to be recalculated
    vector<int32_t> CurrentSeam;
    CurrentSeam.resize(numRows_);

    // declare/initialize variables used in CurrentSeam discovery when looking for the least
    //      cumulative energy column in the bottom row
    double minTotalEnergy = posInf_;
    int32_t minTotalEnergyCol = -1;

    /*** RUN SEAM DISCOVERY ***/
    for (int32_t n = 0; n < numSeams; n++)
    {
        // find least cumulative energy column in bottom row
        // initialize total energy to +INF and run linear search for a pixel of least cumulative
        //      energy (if one exists)
        minTotalEnergy = posInf_;
        minTotalEnergyCol = -1;
        for (int32_t Column = 0; Column < numColumns_; Column++)
        {
            if (!markedPixels[bottomRow_][Column] &&
                TotalEnergyTo[bottomRow_][Column] < minTotalEnergy)
            {
                minTotalEnergy = TotalEnergyTo[bottomRow_][Column];
                minTotalEnergyCol = Column;
            }
        }

        // all pixels in bottom row are unreachable due to +INF cumulative energy to all of them
        // therefore need to recalculate cumulative energies
        if (minTotalEnergyCol == -1)
        {
            // decrement CurrentSeam number iterator since this CurrentSeam was invalid
            // need to recalculate the cumulative energy
            n--;
            SeamRecalculationCount++;
            this->calculateCumulativeVerticalPathEnergy(pixelEnergy, TotalEnergyTo, ColumnTo);
            std::cout << "recalculated seam number: " << n + 1 << std::endl;
            goto ContinueSeamFindingLoop;
        }

        // save last column as part of CurrentSeam
        CurrentSeam[bottomRow_] = minTotalEnergyCol;

        int32_t col = minTotalEnergyCol;
        int32_t currentCol = col;
        for (int32_t Row = bottomRow_ - 1; Row >= 0; Row--)
        {
            // using the below pixel's row and column, extract the column of the pixel in the
            //      current row
            currentCol = ColumnTo[Row + 1][col];

            // check if the current seam we're swimming up has a pixel that has been used part of
            //      another seam
            if (markedPixels[Row][currentCol])
            {
                // mark the starting pixel in bottom row as having +INF cumulative energy so it
                //      will not be chosen again
                TotalEnergyTo[bottomRow_][minTotalEnergyCol] = posInf_;
                // decrement CurrentSeam number iterator since this CurrentSeam was invalid
                n--;
                // restart CurrentSeam finding loop
                goto ContinueSeamFindingLoop;
            }

            // save the column of the pixel in the current row
            CurrentSeam[Row] = currentCol;

            // update to current column
            col = currentCol;
        }

        // copy CurrentSeam and mark appropriate pixels
        for (int32_t Row = 0; Row < numRows_; Row++)
        {
            col = CurrentSeam[Row];
            outDiscoveredSeams[Row].push(col);
            markedPixels[Row][col] = true;
        }

        ContinueSeamFindingLoop:
        {
            continue;
        }
    }
    std::cout << "recalculated total times: " << SeamRecalculationCount << std::endl;
    return true;
}


void cv::SeamCarver::calculateCumulativeVerticalPathEnergy(
    const vector<vector<double>>& pixelEnergy,
    vector<vector<double>>& outTotalEnergyTo,
    vector<vector<int32_t>>& outColumnTo)
{
    // initialize top row
    for (int32_t Column = 0; Column < numColumns_; Column++)
    {
        // if previously markedPixels, set its energy to +INF
        if (markedPixels[0][Column])
        {
            outTotalEnergyTo[0][Column] = posInf_;
        }
        else
        {
            outTotalEnergyTo[0][Column] = this->marginEnergy;
        }
        outColumnTo[0][Column] = -1;
    }

    // cache the total energy to the pixels up/left, directly above, and up/right
    //   instead of accessing memory for the same pixels
    // shift energy values to the left and access memory only once
    // SHIFT OPERATION:
    //   left/above <== directly above
    //   directly above <== right/above
    //   right/above = access new memory
    double energyUpLeft = posInf_;
    double energyUp = posInf_;
    double energyUpRight = posInf_;

    bool markedUpLeft = false;
    bool markedUp = false;
    bool markedUpRight = false;

    double minEnergy = posInf_;
    int32_t minEnergyCol = -1;

    for (int32_t Row = 1; Row < numRows_; Row++)
    {
        energyUpLeft = posInf_;
        energyUp = outTotalEnergyTo[Row - 1][0];
        energyUpRight = numColumns_ > 1 ? outTotalEnergyTo[Row - 1][1] : posInf_;

        markedUpLeft = true;
        markedUp = markedPixels[Row - 1][0];
        markedUpRight = numColumns_ > 1 ? markedPixels[Row - 1][1] : true;

        // find minimum energy path from previous row to every pixel in the current row
        for (int32_t Column = 0; Column < numColumns_; Column++)
        {
            // initialize min energy to +INF and initialize the previous column to -1
            //   to set error state
            minEnergy = posInf_;
            minEnergyCol = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously markedPixels
            if (!markedPixels[Row][Column])
            {
                // check above
                if (!markedUp && energyUp < minEnergy)
                {
                    minEnergy = energyUp;
                    minEnergyCol = Column;
                }

                // check if right/above is min
                if (Column < numColumns_ - 1)
                {
                    if (!markedUpRight && energyUpRight < minEnergy)
                    {
                        minEnergy = energyUpRight;
                        minEnergyCol = Column + 1;
                    }
                }

                // check if left/above is min
                if (Column > 0)
                {
                    if (!markedUpLeft && energyUpLeft < minEnergy)
                    {
                        minEnergy = energyUpLeft;
                        minEnergyCol = Column - 1;
                    }
                }
            }

            // shift energy to the left
            energyUpLeft = energyUp;
            markedUpLeft = markedUp;
            energyUp = energyUpRight;
            markedUp = markedUpRight;

            // get markedPixels and TotalEnergyTo data for pixels right/above
            if (numColumns_ > 1 && Column < numColumns_ - 2)
            {
                energyUpRight = outTotalEnergyTo[Row - 1][Column + 2];
                markedUpRight = markedPixels[Row - 1][Column + 2];
            }

            // assign cumulative energy to current pixel and save the column of the parent pixel
            if (minEnergyCol == -1)
            {
                // current pixel is unreachable from parent pixels since they are all markedPixels
                //   OR current pixel already markedPixels
                // set energy to reach current pixel to +INF
                outTotalEnergyTo[Row][Column] = posInf_;
            }
            else
            {
                outTotalEnergyTo[Row][Column] = minEnergy + pixelEnergy[Row][Column];
            }
            outColumnTo[Row][Column] = minEnergyCol;
        }
    }
}


void cv::SeamCarver::removeVerticalSeams(vector<cv::Mat>& bgr, vectorOfMinPQ& seams)
{
    // each row of seams stores an ordered queue of pixels to remove in that row
    //   starting with the min number column
    // each time a new column is encountered, move the pixels to the right of it
    //   (up until the next column number) to the left by the number of pixels already removed

    int32_t colToRemove = 0;
    int32_t numSeamsRemoved = 0;
    /*** REMOVE PIXELS FOR EVERY ROW ***/
    for (int32_t r = 0; r < numRows_; r++)
    {
        // SeamRecalculationCount the number of seams to the left of the current pixel
        //   to indicate how many spaces to move pixels that aren't being removed to the left
        numSeamsRemoved = 0;
        // loop through all pixels to remove in current row
        while (seams[r].size())
        {
            numSeamsRemoved++;
            // column location of pixel to remove in row Row
            colToRemove = seams[r].pop();
            //seams[Row].pop();
            // mark right endpoint/next pixel column
            int32_t rightColBorder = (seams[r].empty() ? numColumns_ : seams[r].top());
            // starting at the column to the right of the column to remove,
            //      move the pixel to the left by the number of seams to the left of the pixel,
            //      until the right end point which is either the last column or the next column
            //      to remove whichever comes first
            for (int c = colToRemove + 1; c < rightColBorder; c++)
            {
                for (int32_t j = 0; j < 3; j++)
                {
                    bgr[j].at<uchar>(r, c - numSeamsRemoved) = bgr[j].at<uchar>(r, c);
                }
            }
        }
    }

    /*** SHRINK IMAGE BY REMOVING SEAMS ***/
    // TODO remove magic number
    int32_t NumColorChannels = 3;
    for (int32_t Channel = 0; Channel < NumColorChannels; Channel++)
    {
        bgr[Channel] = bgr[Channel].colRange(0, bgr[Channel].cols - numSeamsRemoved);
    }
}
