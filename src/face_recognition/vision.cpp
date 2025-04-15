#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/face.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/utils/logger.hpp>


#include "vision.hpp"
#include "client.hpp"
#include "serial_port.hpp"

const std::string FACES_PATH = "../src/face_recognition/faces/";
const std::string CASCADES_PATH = "../src/face_recognition/Cascades/";
using namespace cv;
using namespace cv::face;

struct tm y2k = {0};
const int NUM_PERSONS = 6;
const std::string MODEL_PATH = "model.xml";


Mat preprocessImage(Mat image,Size imageSize){
    Mat gray_image, image_resized;
    cvtColor(image, gray_image, COLOR_BGR2GRAY);
    resize(gray_image, image_resized, imageSize);
    return image_resized;
}
Mat getImage(Size& imageSize)
{
    std::string imgBinary = client::getImage();
    std::vector<uchar> data(imgBinary.begin(), imgBinary.end());
    Mat image = imdecode(data, 1);

    return image;
}
int loadCascade(CascadeClassifier& faceCascade,CascadeClassifier& eyesCascade){
    std::string faceCascadeName = CASCADES_PATH + "haarcascade_frontalface_alt.xml";
    std::string eyesCascadeName = CASCADES_PATH + "haarcascade_eye_tree_eyeglasses.xml";
    if( !faceCascade.load( faceCascadeName ) ){ printf("--(!)Error loading 1\n"); return -1; };
    if( !eyesCascade.load( eyesCascadeName ) ){ printf("--(!)Error loading 2\n"); return -1; };

    return 1;
}
std::vector<Rect> detectFaces(Mat& frame,CascadeClassifier& faceCascade,CascadeClassifier& C,Size detectionScaleSize){

    std::vector<Rect> faces;
    Mat frameGray;
    if(frame.channels() != 1){
    cvtColor(frame,frameGray,COLOR_BGR2GRAY);
    }
    else{
        frameGray = frame;
    }
    equalizeHist(frameGray,frameGray);
    faceCascade.detectMultiScale(frameGray,faces,1.1,2,0,detectionScaleSize);
    
    return faces;
}
void displayFaces(std::vector<Rect>& faces,Mat& frame){
    std::string windowName = "cascade";
    for(auto& face:faces){
        Point center( face.x + face.width*0.5, face.y + face.height*0.5 );
        ellipse(frame,center,Size(face.width * 0.5,face.height * 0.5),0,0,360,Scalar(255,0,255),2,8,0);
    }
    imshow(windowName,frame);
}
Ptr<FisherFaceRecognizer> setupFisherFacesModel(Size &image_size,bool isModelSaved,CascadeClassifier& faceCascade,CascadeClassifier& eyesCascade)
{
    Ptr<FisherFaceRecognizer> model;
    if(isModelSaved){
        model = FisherFaceRecognizer::load<FisherFaceRecognizer>(MODEL_PATH);
        return model;
    }

    std::ifstream file(FACES_PATH + std::string("info.csv"), std::ifstream::in);
    if (!file)
    {
        std::string error_message = "No valid input file was given, please check the given filename. " + FACES_PATH + std::string("info.csv");
        CV_Error(Error::StsBadArg, error_message);
    }
    std::vector<Mat> images;
    std::vector<int> labels;
    std::string line, path, classlabel;
    while (std::getline(file, line))
    {
        std::stringstream liness(line);
        std::getline(liness, path, ',');
        std::getline(liness, classlabel);
        if (!path.empty() && !classlabel.empty())
        {
            Mat image = imread(FACES_PATH + path, 0);
            Mat image_resized;
            resize(image, image_resized, image_size);
            images.push_back(image_resized);
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
    const int CONFIDENCE_THRESHOLD = 3000;
    model = FisherFaceRecognizer::create(0,CONFIDENCE_THRESHOLD);
    model->train(images, labels);
    model->save(MODEL_PATH);
    return model;
}
int patrol(time_t& last_time, SerialPort &arduino,int curr_pos){
    time_t current_time;

    time(&current_time);  /* get current time; same as: timer = time(NULL)  */

    int seconds = difftime(current_time,last_time);
    if(seconds > 10){
        last_time = current_time;
        arduino.write("1");
        return curr_pos + 1;
    };
    return curr_pos;
}
int centerFace(){
    return -1;
}
void automatedMode(bool isModelSaved)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
    Size imageSize(500, 500);
    std::string windowName = "test";

    CascadeClassifier faceCascade;
    CascadeClassifier eyeCascade;
    loadCascade(faceCascade,eyeCascade);
    Ptr<FisherFaceRecognizer> model = setupFisherFacesModel(imageSize,isModelSaved,faceCascade,eyeCascade);
    //SerialPort arduinoPort = loadServo();

    int framesDetected = 0;
    int currentLabel = 0;

    const int DETECTION_THRESHOLD = 20;
    time_t last_time;
    time(&last_time);
    int curr_pos = 0;

    std::vector<int> framesDetectedPerPerson(NUM_PERSONS);
    while (true)
    {
        if (waitKey(10) > 10)
            break;
        Mat image = getImage(imageSize);
        //capture.read(image);
        //detectAndDisplay(image,faceCascade,eyeCascade);
        imshow(windowName,image);
        image = preprocessImage(image,imageSize);
        int predicted_label = -1;
        double confidence = 0.0;
        model->predict(image, predicted_label, confidence);
        
        std::cout << predicted_label << " " << confidence <<" "<<framesDetected<<std::endl;
        
        if(predicted_label != -1) framesDetectedPerPerson[predicted_label-1]++;
        if(*max_element(framesDetectedPerPerson.begin(),framesDetectedPerPerson.end()) > DETECTION_THRESHOLD){
            std::cout<<"Person detected from the team\n";
        }
        else{
            time_t current_time;
            time(&current_time);
            if(difftime(current_time,last_time) > 5){
                framesDetectedPerPerson.assign(NUM_PERSONS,0);
                last_time = 0;
            }
        }
        
    }

}
