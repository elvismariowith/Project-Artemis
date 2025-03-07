#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <vector>
#include <cstddef>

#include "vision.hpp"
#include "client.hpp"

using namespace cv;

std::vector<Rect> detectPerson(Mat &frame, HOGDescriptor &hog)
{

    std::vector<Rect> personsDetected;
    std::vector<double> weights;
    Mat frameGray;
    equalizeHist(frameGray,frameGray);
    cvtColor(frame,frameGray,COLOR_BGR2GRAY);
    hog.detectMultiScale(frame, personsDetected, weights,0,Size(2,2));
    return personsDetected;
}

void automatedMode()
{
    std::string windowName = "test";
    auto hog = HOGDescriptor::HOGDescriptor();
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    VideoCapture capture(0);
    if (capture.isOpened())
    {
        while (true)
        {
            if(waitKey(1) > 10) break;
            std::string imgBinary = client::getImage();
            std::vector<uchar> data(imgBinary.begin(),imgBinary.end());
            Mat image = imdecode(data,1);
            std::vector<Rect> persons = detectPerson(image, hog);
            for (auto &person : persons)
            {
                Point center(person.x + person.width * 0.5, person.y + person.height * 0.5);
                ellipse(image, center, Size(person.width * 0.5, person.height * 0.5), 0, 0, 360, Scalar(255, 0, 255), 2, 8, 0);
            }
            imshow(windowName, image);
        }
    }
}
