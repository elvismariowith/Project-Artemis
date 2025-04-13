#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/face.hpp>

#include "vision.hpp"
#include "client.hpp"

const std::string data_path = "../src/face_recognition/faces/";
using namespace cv;
using namespace cv::face;

Ptr<EigenFaceRecognizer> setupEigenFacesModel(Size& image_size){

    std::ifstream file(data_path + std::string("info.csv"), std::ifstream::in);
    if (!file) {
        std::string error_message = "No valid input file was given, please check the given filename. " + data_path + std::string("info.csv");
        CV_Error(Error::StsBadArg, error_message);
    }
    std::vector<Mat> images;
    std::vector<int> labels;
    std::string line, path, classlabel;
    while (std::getline(file, line)) {
        std::stringstream liness(line);
        std::getline(liness, path, ',');
        std::getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            Mat image = imread(data_path+path,0);
            Mat image_resized;
            resize(image,image_resized,image_size);
            images.push_back(image_resized);
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
    Ptr<EigenFaceRecognizer> model = EigenFaceRecognizer::create();
    model->train(images, labels);
    return model;
}

void automatedMode()
{
    Size image_size(500,500);
    std::string windowName = "test";
    Ptr<EigenFaceRecognizer> model = setupEigenFacesModel(image_size);
    if (capture.isOpened())
    {
        while (true)
        {
            if(waitKey(1) > 10) break;
            std::string imgBinary = client::getImage();
            std::vector<uchar> data(imgBinary.begin(),imgBinary.end());
            Mat image = imdecode(data,1);
            Mat gray_image,image_resized;
            imshow(windowName, image);
            cvtColor(image,gray_image,COLOR_BGR2GRAY);
            resize(gray_image,image_resized,image_size);
            int predicted_label = -1;
            double confidence = 0.0;
            model->predict(image_resized,predicted_label,confidence);
        
            std::cout<<predicted_label<<" "<<confidence<<'\n';
        }
    }
}
