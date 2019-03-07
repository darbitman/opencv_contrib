#include "opencv2/seamcarver/seamcarver.hpp"

cv::SeamCarver::SeamCarver(double marginEnergy) :
    marginEnergy_(marginEnergy),
    pixelEnergyCalculator_(marginEnergy)
{
}

cv::SeamCarver::SeamCarver(size_t numRows, size_t numColumns, double marginEnergy) :
    marginEnergy_(marginEnergy),
    pixelEnergyCalculator_(marginEnergy)
{
    init(numRows, numColumns);
}

void cv::SeamCarver::runVerticalSeamRemover(size_t numSeams,
                                            const cv::Mat& img,
                                            cv::Mat& outImg,
                                            cv::energyFunc computeEnergyFunction)
{
    if (needToInitializeLocalData)
    {
        init(img);
    }

    // check if removing more seams than columns available
    if (numSeams > numColumns_)
    {
        CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
    }

    resetLocalVectors(numSeams);

    findAndRemoveVerticalSeams(numSeams, img, outImg, computeEnergyFunction);
}

void cv::SeamCarver::init(const cv::Mat& img)
{
    init(img.rows, img.cols);
}

void cv::SeamCarver::init(size_t numRows, size_t numColumns)
{
    initializeLocalVariables(numRows, numColumns, numRows - 1, numColumns - 1);
    initializeLocalVectors();
    needToInitializeLocalData = false;
}

inline void cv::SeamCarver::initializeLocalVariables(size_t numRows, size_t numColumns,
                                                     size_t bottomRow, size_t rightColumn)
{
    numRows_ = numRows;
    numColumns_ = numColumns;
    bottomRow_ = bottomRow;
    rightColumn_ = rightColumn;
}

void cv::SeamCarver::initializeLocalVectors()
{
    pixelEnergy.resize(numRows_);
    markedPixels.resize(numRows_);
    totalEnergyTo.resize(numRows_);
    columnTo.resize(numRows_);
    currentSeam.resize(numRows_);

    for (size_t row = 0; row < numRows_; row++)
    {
        pixelEnergy[row].resize(numColumns_);
        markedPixels[row].resize(numColumns_);
        totalEnergyTo[row].resize(numColumns_);
        columnTo[row].resize(numColumns_);
    }

    bgr.resize(pixelEnergyCalculator_.getDimensions().numColorChannels_);

    discoveredSeams.resize(numRows_);
}

void cv::SeamCarver::findAndRemoveVerticalSeams(const size_t& numSeams,
                                                const cv::Mat& img,
                                                cv::Mat& outImg,
                                                const cv::energyFunc computeEnergyFunction)
{
    // check if removing more seams than columns available
    if (numSeams > numColumns_)
    {
        CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
    }

    cv::split(img, bgr);

    try
    {
        // Compute pixel energy
        if (computeEnergyFunction == nullptr)
        {
            pixelEnergyCalculator_.setDimensions(numColumns_, numRows_, img.channels());
            pixelEnergyCalculator_.calculatePixelEnergy(img, pixelEnergy);
        }
        else
        {
            // call user-defined energy computation function
            computeEnergyFunction(img, pixelEnergy);
        }

        // find all vertical seams
        findVerticalSeams(numSeams);

        // remove all found seams, least cumulative energy first
        removeVerticalSeams();

        // combine separate channels into output image
        cv::merge(bgr, outImg);
    }
    catch (...)
    {
        // TODO handle exception
    }
}


void cv::SeamCarver::resetLocalVectors(size_t numSeams)
{
    for (size_t row = 0; row < numRows_; row++)
    {
        // set marked pixels to false for new run
        for (size_t column = 0; column < numColumns_; column++)
        {
            markedPixels[row][column] = false;
        }

        // ensure priority queue has at least numSeams capacity
        if (numSeams > discoveredSeams[row].capacity())
        {
            discoveredSeams[row].allocate(numSeams);
        }

        // reset priority queue since it could be filled from a previous run
        if (discoveredSeams[row].size() > 0)
        {
            discoveredSeams[row].resetHeap();
        }
    }
}

void cv::SeamCarver::findVerticalSeams(size_t numSeams)
{
    if (pixelEnergy.size() == 0)
    {
        CV_Error(Error::Code::StsInternal,
                 "SeamCarver::findVerticalSeams() failed due to zero-size pixelEnergy vector");
    }

    if (discoveredSeams.size() != pixelEnergy.size())
    {
        CV_Error(Error::Code::StsInternal,
                 "SeamCarver::findVerticalSeams() failed due to different sized vectors");
    }

    // initial cumulative energy path calculation
    calculateCumulativeVerticalPathEnergy();

    // declare/initialize variables used in currentSeam discovery when looking for the least
    //      cumulative energy column in the bottom row
    double minTotalEnergy = posInf_;
    int32_t minTotalEnergyCol = -1;
    bool restartSeamDiscovery = false;   // seam discovery needs to be restarted for currentSeam

    /*** RUN SEAM DISCOVERY ***/
    for (size_t n = 0; n < numSeams; n++)
    {
        // initialize total energy to +INF and run linear search for a pixel of least cumulative
        //      energy (if one exists) in the bottom row
        minTotalEnergy = posInf_;
        minTotalEnergyCol = -1;
        for (size_t column = 0; column < numColumns_; column++)
        {
            if (!markedPixels[bottomRow_][column] &&
                totalEnergyTo[bottomRow_][column] < minTotalEnergy)
            {
                minTotalEnergy = totalEnergyTo[bottomRow_][column];
                minTotalEnergyCol = column;
            }
        }

        // all pixels in bottom row are unreachable due to +INF cumulative energy to all of them
        // therefore need to recalculate cumulative energies
        if (minTotalEnergyCol == -1)
        {
            // decrement currentSeam number iterator since this currentSeam was invalid
            // need to recalculate the cumulative energy
            n--;
            calculateCumulativeVerticalPathEnergy();

            restartSeamDiscovery = true;
        }

        // save last column as part of currentSeam that will be checked whether it can fully reach
        //      the top row
        currentSeam[bottomRow_] = minTotalEnergyCol;

        // initialize variables to keep track of columns
        size_t prevCol = minTotalEnergyCol;
        size_t currentCol = prevCol;

        if (!restartSeamDiscovery)
        {
            for (size_t row = bottomRow_ - 1; row >= 0; row--)
            {
                // using the below pixel's row and column, extract the column of the pixel in the
                //      current row
                currentCol = columnTo[row + 1][prevCol];

                // check if the current pixel of the current seam has been used part of another seam
                if (markedPixels[row][currentCol])
                {
                    // mark the starting pixel in bottom row as having +INF cumulative energy so it
                    //      will not be chosen again
                    totalEnergyTo[bottomRow_][minTotalEnergyCol] = posInf_;
                    // decrement currentSeam number iterator since this currentSeam was invalid
                    n--;
                    // restart currentSeam finding loop
                    restartSeamDiscovery = true;
                    break;
                }

                // save the column of the pixel in the current row
                currentSeam[row] = currentCol;

                // save current column to be used for the next iteration of the loop
                prevCol = currentCol;
            }
        }

        if (restartSeamDiscovery)
        {
            restartSeamDiscovery = false;
            continue;
        }
        else
        {
            // copy currentSeam and mark appropriate pixels
            for (size_t row = 0; row < numRows_; row++)
            {
                prevCol = currentSeam[row];
                discoveredSeams[row].push(prevCol);
                markedPixels[row][prevCol] = true;
            }
        }
    }   // for (size_t n = 0; n < numSeams; n++)
}


void cv::SeamCarver::calculateCumulativeVerticalPathEnergy(
    //const vector<vector<double>>& pixelEnergy,
    //vector<vector<double>>& outTotalEnergyTo,
    //vector<vector<int32_t>>& outColumnTo
)
{
    // initialize top row
    for (size_t column = 0; column < numColumns_; column++)
    {
        // if previously markedPixels, set its energy to +INF
        if (markedPixels[0][column])
        {
            totalEnergyTo[0][column] = posInf_;
        }
        else
        {
            totalEnergyTo[0][column] = marginEnergy_;
        }
        columnTo[0][column] = -1;
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

    for (size_t row = 1; row < numRows_; row++)
    {
        energyUpLeft = posInf_;
        energyUp = totalEnergyTo[row - 1][0];
        energyUpRight = numColumns_ > 1 ? totalEnergyTo[row - 1][1] : posInf_;

        markedUpLeft = true;
        markedUp = markedPixels[row - 1][0];
        markedUpRight = numColumns_ > 1 ? markedPixels[row - 1][1] : true;

        // find minimum energy path from previous row to every pixel in the current row
        for (size_t column = 0; column < numColumns_; column++)
        {
            // initialize min energy to +INF and initialize the previous column to -1
            //   to set error state
            minEnergy = posInf_;
            minEnergyCol = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously markedPixels
            if (!markedPixels[row][column])
            {
                // check above
                if (!markedUp && energyUp < minEnergy)
                {
                    minEnergy = energyUp;
                    minEnergyCol = column;
                }

                // check if right/above is min
                if (column < numColumns_ - 1)
                {
                    if (!markedUpRight && energyUpRight < minEnergy)
                    {
                        minEnergy = energyUpRight;
                        minEnergyCol = column + 1;
                    }
                }

                // check if left/above is min
                if (column > 0)
                {
                    if (!markedUpLeft && energyUpLeft < minEnergy)
                    {
                        minEnergy = energyUpLeft;
                        minEnergyCol = column - 1;
                    }
                }
            }

            // shift energy to the left
            energyUpLeft = energyUp;
            markedUpLeft = markedUp;
            energyUp = energyUpRight;
            markedUp = markedUpRight;

            // get markedPixels and totalEnergyTo data for pixels right/above
            if (numColumns_ > 1 && column < numColumns_ - 2)
            {
                energyUpRight = totalEnergyTo[row - 1][column + 2];
                markedUpRight = markedPixels[row - 1][column + 2];
            }

            // assign cumulative energy to current pixel and save the column of the parent pixel
            if (minEnergyCol == -1)
            {
                // current pixel is unreachable from parent pixels since they are all markedPixels
                //   OR current pixel already markedPixels
                // set energy to reach current pixel to +INF
                totalEnergyTo[row][column] = posInf_;
            }
            else
            {
                totalEnergyTo[row][column] = minEnergy + pixelEnergy[row][column];
            }
            columnTo[row][column] = minEnergyCol;
        }
    }
}


void cv::SeamCarver::removeVerticalSeams(//vector<cv::Mat>& bgr,
                                         //vectorOfMinOrientedPQ& seams
)
{
    // each row of seams stores an ordered queue of pixels to remove in that row
    //   starting with the min number column
    // each time a new column is encountered, move the pixels to the right of it
    //   (up until the next column number) to the left by the number of pixels already removed

    int32_t colToRemove = 0;
    int32_t numSeamsRemoved = 0;
    /*** REMOVE PIXELS FOR EVERY ROW ***/
    for (size_t r = 0; r < numRows_; r++)
    {
        // seamRecalculationCount the number of seams to the left of the current pixel
        //   to indicate how many spaces to move pixels that aren't being removed to the left
        numSeamsRemoved = 0;
        // loop through all pixels to remove in current row
        while (discoveredSeams[r].size())
        {
            numSeamsRemoved++;
            // column location of pixel to remove in row row
            colToRemove = discoveredSeams[r].pop();
            //seams[row].pop();
            // mark right endpoint/next pixel column
            size_t rightColBorder = (discoveredSeams[r].empty() ?
                                     numColumns_ : discoveredSeams[r].top());
            // starting at the column to the right of the column to remove,
            //      move the pixel to the left by the number of seams to the left of the pixel,
            //      until the right end point which is either the last column or the next column
            //      to remove whichever comes first
            for (size_t c = colToRemove + 1; c < rightColBorder; c++)
            {
                for (int32_t j = 0; j < 3; j++)
                {
                    bgr[j].at<uchar>(r, c - numSeamsRemoved) = bgr[j].at<uchar>(r, c);
                }
            }
        }
    }

    /*** SHRINK IMAGE BY REMOVING SEAMS ***/
    int32_t numColorChannels = pixelEnergyCalculator_.getDimensions().numColorChannels_;
    for (int32_t Channel = 0; Channel < numColorChannels; Channel++)
    {
        bgr[Channel] = bgr[Channel].colRange(0, bgr[Channel].cols - numSeamsRemoved);
    }
}
