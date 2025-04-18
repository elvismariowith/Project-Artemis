#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <thread>

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

const int NUM_PERSONS = 6;
const std::string MODEL_PATH = "model.xml";
const int PATROL_COMMAND = 9;
const int SHOOT_COMMAND = 3;
const int STOP_SHOOT_COMMAND = -3;
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
 	    std::cout<<FACES_PATH + path<<std::endl;
            Mat image = imread(FACES_PATH + path, 0);
            std::cout<<image.rows<<std::endl;
            Mat image_resized;
            resize(image, image_resized, image_size);
            images.push_back(image_resized);
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
    const int CONFIDENCE_THRESHOLD = 2500;
    model = FisherFaceRecognizer::create(0,CONFIDENCE_THRESHOLD);
    model->train(images, labels);
    model->save(MODEL_PATH);
    std::cout<<"Model loaded successfully\n";
    return model;
}
void patrol(SerialPort &arduinoPort) {
    
    int random_direction = (int)rand() %  2;
    int command = (random_direction == 0) ? 1 : -1;

    for(int i = 0; i < 100; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        arduinoPort.write(command);
    }

    std::cout<<"Patrolling "<<command<<'\n';
}
std::pair<int,int> getDirectionToMove(int pointx,int pointy,Rect& rectangleFace) {
    std::pair<int,int> direction = {0,0};
    int OFFSET_HORIZONTAL = rectangleFace.width / 3;
    std::cout<<"You are at "<<rectangleFace.y<<" with width "<<rectangleFace.height<<'\n';

    if(rectangleFace.x + OFFSET_HORIZONTAL > pointx){
        direction.first = 1;
    }
    else if(rectangleFace.x + rectangleFace.width - OFFSET_HORIZONTAL > pointx && pointx > rectangleFace.x + OFFSET_HORIZONTAL){
        direction.first = 0;
    }
    else{
        direction.first = -1;
    }
    if(rectangleFace.y > pointy){
        direction.second = 2;
    }
    else if(rectangleFace.y + rectangleFace.height > pointy && pointy > rectangleFace.y){
        direction.second = 0;
    }
    else{
	direction.second = -2;
    }
    return direction;
}
int centerFace(Size& imageSize,CascadeClassifier& faceCascade,CascadeClassifier& eyesCascade,SerialPort& arduinoPort){
    int framesItCanFail = 10;

    Mat image = getImage(imageSize);
    Size detectionSize = Size(30,30);
    std::vector<Rect> faces;
    const int OFFSET_VERTICAL = 100;
    int centerx = image.rows / 2;
    int centery = image.cols / 2 - OFFSET_VERTICAL;
    std::pair<int,int> direction;
    while(framesItCanFail  > 0){
        image = getImage(imageSize);
        faces = detectFaces(image,faceCascade,eyesCascade,detectionSize);
        std::cout<<framesItCanFail<<'\n';
        if(faces.size() == 0){
            framesItCanFail--;
            continue;
        }
        direction = getDirectionToMove(centerx,centery,faces[0]);
        std::cout<<direction.first<<" "<<direction.second<<'\n';
        if(direction == std::make_pair(0,0)) return 1;
        if(direction.first != 0){
            arduinoPort.write(direction.first);
        }
        if(direction.second != 0){
            arduinoPort.write(direction.second);
        }

    }
    return 0;
}
void shoot(SerialPort& arduinoPort){
    std::cout<<"Shooting Daniel "<<'\n';
    arduinoPort.write(SHOOT_COMMAND);
    arduinoPort.write(STOP_SHOOT_COMMAND);
}
void automatedMode(bool isModelSaved)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
    Size imageSize(350, 350);
    std::string windowName = "test";

    CascadeClassifier faceCascade;
    CascadeClassifier eyeCascade;
    loadCascade(faceCascade,eyeCascade);
    Ptr<FisherFaceRecognizer> model = setupFisherFacesModel(imageSize,isModelSaved,faceCascade,eyeCascade);
    SerialPort arduinoPort = findArduinoSerialPort();

    int framesDetected = 0;
    int currentLabel = 0;

    const int DETECTION_THRESHOLD = 5;
    const int WAIT_TIME_S = 15;
    time_t last_time;
    time_t last_patrol_time;
    time(&last_time);
    time(&last_patrol_time);
    int curr_pos = 0;

    std::vector<int> framesDetectedPerPerson(NUM_PERSONS);

    while (true) {
        patrol(arduinoPort);
    }

    while (true)
    {
        
        time_t current_time;
        time(&current_time);
        if (waitKey(10) > 10)
            break;
        Mat image = getImage(imageSize);
        //capture.read(image);
        //detectAndDisplay(image,faceCascade,eyeCascade);
        std::vector<Rect> faces = detectFaces(image, faceCascade, eyeCascade, Size(100, 100));
        if (faces.empty()){
	     if(difftime(current_time,last_patrol_time) > 3) {
		    patrol(arduinoPort);
		    last_patrol_time = current_time;
	     }
	     continue;
        }

        //displayFaces(faces,image);
        cv::Size inflationSize(50,50);
        faces[0] += inflationSize;

        Rect imageRect = Rect(0,0,image.cols,image.rows);
        Mat faceROI = image(faces[0] & imageRect); // take first detected face
       //imshow("test",faceROI);
        faceROI = preprocessImage(image,imageSize); // match training size



        int predicted_label = -1;
        double confidence = 0.0;
        model->predict(faceROI, predicted_label, confidence);
        std::cout << predicted_label << " " << confidence <<" "<<framesDetected<<std::endl;
        
        
        if(predicted_label != -1) framesDetectedPerPerson[predicted_label-1]++;
        
        if(*max_element(framesDetectedPerPerson.begin(),framesDetectedPerPerson.end()) > DETECTION_THRESHOLD){
            std::cout<<"Person detected from the team\n";
            int couldCenter = centerFace(imageSize,faceCascade,eyeCascade,arduinoPort);
            if(couldCenter){
		framesDetectedPerPerson.assign(NUM_PERSONS,0);
                shoot(arduinoPort);
            }
        }
        else{
	    double diff = difftime(current_time,last_time);
            std::cout<<diff<<'\n';
            if(diff > WAIT_TIME_S){
                framesDetectedPerPerson.assign(NUM_PERSONS,0);
                last_time = current_time;
            }
        }

    }

}
