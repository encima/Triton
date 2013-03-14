#include <dirent.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "/usr/local/include/opencv2/opencv.hpp"
#include "/usr/local/include/cvblob.h"

#include "DirTraveler.h"

using namespace std;
using namespace cvb;
using namespace cv;

int step = 3;

vector<string> split(string s, char delim) 
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    //getline reads in chars from ss, stores them in item, until the delim has been found.
    while(getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

string splitPath(string file, bool pathOrFile)
{
    unsigned found = file.find_last_of("/");
    if(pathOrFile) 
    {
        string path = file.substr(0, found);
        return path;
    }else{
        string fileName = file.substr(found+1);
        return fileName;
    }
}

void identifyCvBlobs(Mat *fore, string path) 
{
    IplImage fg = *fore;
    IplImage *labelImg = cvCreateImage(cvGetSize(&fg), IPL_DEPTH_LABEL, 1);
    // IplImage *output = cvCreateImage(cvGetSize(&fg), IPL_DEPTH_8U, 3);
    
    CvBlobs blobs;
    CvTracks tracks;

    //Potential memory leak, this is never released (should be resolved with the cvReleaseBlobs)
    unsigned int result = cvLabel(&fg, labelImg, blobs);

    cout << "Blobs found" << blobs.size() << endl;
    if(blobs.size() > 0) {
        CvBlob *largest = blobs[cvGreaterBlob(blobs)];
        
        Rect rect = Rect(largest->minx, largest->miny, largest->maxx-largest->minx, largest->maxy-largest->miny);
        cvReleaseImage(&labelImg);
        //Check if largest blob found is not just the image size
        if(fg.width % rect.width > 10 && fg.height % rect.height > 10)
        {
            cout << "Extracting largest blob from image" << endl;
            cvDrawRect(&fg, cvPoint(rect.x,rect.y), cvPoint(rect.x+rect.width,rect.y+rect.height), CV_RGB(0,0,255), 1, CV_AA, 0);
            cvSetImageROI(&fg, rect);
            Mat result = &fg;
            stringstream procPath;
            procPath << splitPath(path, true) << "/1BGMOG2/" << splitPath(path, false);
            imwrite(procPath.str(), result);
            cout << "***FOUND Interesting Image, saving as:" << procPath.str() << endl;
            // imshow("Extracted", result);
            // imshow("Original", *fore);
            // cvWaitKey();
        }else{
            printf("Image was either too busy, or empty. Nothing extracted. \n");
        }
        cvReleaseBlobs(blobs);
        cvReleaseTracks(tracks);
    }else{
        printf("No blobs found, nothing written\n");
    }
}

void createBGMod(vector<string> *images)
{
    Mat frame, fore, cropped;
    BackgroundSubtractorMOG2 bgMod;
    vector<vector<Point> > contours;
    cout << "Processing Set: " << (*images)[0] << endl;
    for(int i = 0; i < images->size(); i++)
    {
        //load image and scale out the reconyx bars
        frame = imread((*images)[i], CV_LOAD_IMAGE_COLOR);
        if(!frame.empty()) {
            Rect roi(0, 30, frame.size().width, (frame.size().height-60));
            cropped = frame(roi);
            bgMod.operator()(cropped, fore);
            erode(fore, fore, Mat());
            dilate(fore, fore, Mat());
        }
        //Inverts and draws contours, makes finding blobs harder!
        // cv::findContours(fore, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        // cv::drawContours(cropped, contours, -1, cv::Scalar(0,0,255), 2);
    }
    // Memory leak occurs within the background mod, should be fixed here.
    frame.release();
    cropped.release();
    contours.clear();
    bgMod.~BackgroundSubtractorMOG2();
    identifyCvBlobs(&fore, (*images)[0]);
}

//Don't forget that pesky space before right caret, needs it for 2D vectors. Frick knows why.
vector<vector<string> > sortFiles(vector<string> *files) 
{
    vector<vector<string> > imgDirs;
    vector<string> imgs;
    // string pathPrev = splitPath((*files)[0], true);
    int count = 0;
    cout << "Sorting directories" << endl;
    if((*files).size() > 0) {
        imgs.push_back((*files)[0]);
        for(int i = 1; i < files->size(); i++) 
        {
            // (*files) dereferences the pointer so that we can access the element within the vector.
            // We then split the string at the last instance of the delimiter, to get the path to the file.
            string path = splitPath((*files)[i], true);
            string pathPrev = splitPath((*files)[i-1], true);
            //Compare current path with the one from before, create a new vector if we are in a new directory.
            if(path == pathPrev) {
                imgs.push_back((*files)[i]);
            }else{
                imgDirs.push_back(imgs);
                imgs.clear();
                count++;
                imgs.push_back((*files)[i]);
            }
        }
    }
   //Only one dir found, add just the contents of that dir and empty the vector.
    if(count == 0) {
        cout << "One directory of images found, adding now" << endl;
        imgDirs.push_back(imgs);
        imgs.clear();
    } 

    return imgDirs;
}

int main(int argc, char const *argv[])
{
    if(argc == 2)
    {
        DirTraveler dt;
        vector<string> results;
        vector<string> images;
        dt.travelDirectoryRecursive(argv[1], &results);
        for(int i = 0; i < results.size(); i++)
        {
            //Look for JPG extension, but ignore files in the processed folder
            if(results[i].find("JPG") != string::npos && string::npos == results[i].find("1BGMOG") && string::npos == results[i].find("1Processed"))
            {
                images.push_back(results[i]);
            }
        }
        results.clear();

        vector<vector<string> > imgDirs = sortFiles(&images);
        cout << imgDirs.size() << endl;
        images.clear();

        for(int i = 0; i < imgDirs.size(); i++) 
        {
            stringstream makeProcPath;
            makeProcPath << "mkdir " << splitPath(imgDirs[i][0], true) << "/1BGMOG2/";
            system(makeProcPath.str().c_str());
            cout << "Making Dir: " << makeProcPath.str() << endl;
            for(int j = 0; j < imgDirs[i].size(); (j+=step)) 
            {
                //Split the files into groups of step size (i.e. 3) before processing
                vector<string>::const_iterator first = imgDirs[i].begin() + j;    
                vector<string>::const_iterator last;
                if((j + step) < imgDirs[i].size()) {
                    last = imgDirs[i].begin() + (j + step);
                } else {
                    last = imgDirs[i].begin() + imgDirs[i].size(); 
                }
                vector<string> subset(first, last);
                createBGMod(&subset);
            }
            cout << "-------END OF DIR-------" << endl;
        }
    }
}
