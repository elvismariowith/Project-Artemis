#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "main.hpp"

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------
// Define the face recognition network (this is based on dlib's example)
// ----------------------------------------------------------------------------------------
template <template <int, template<typename> class, int, typename> class block,
          int N, template<typename> class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int, template<typename> class, int, typename> class block,
          int N, template<typename> class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
    ares<256,
    ares_down<256,
    ares<128,
    ares_down<128,
    ares<64,
    ares_down<64,
    relu<affine<con<64,7,7,2,2,
    input_rgb_image_sized<150>
    >>>>>>>>>>>>;

// ----------------------------------------------------------------------------------------
// Helper function: load an image, detect the first face, and compute its encoding.
// ----------------------------------------------------------------------------------------
bool load_face_encoding(const std::string& image_path, 
                        matrix<float,0,1>& face_descriptor,
                        frontal_face_detector& detector,
                        shape_predictor& sp,
                        anet_type& net)
{
    try {
        matrix<rgb_pixel> img;
        load_image(img, image_path);

        std::vector<rectangle> faces = detector(img);
        if (faces.empty()){
            cout << "No face found in " << image_path << endl;
            return false;
        }
        // Use the first detected face
        full_object_detection shape = sp(img, faces[0]);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
        std::vector<matrix<rgb_pixel>> face_chips;
        face_chips.push_back(std::move(face_chip));
        std::vector<matrix<float,0,1>> face_descriptors = net(face_chips);
        if (face_descriptors.empty())
            return false;
        face_descriptor = face_descriptors[0];
        return true;
    } catch (std::exception& e) {
        cerr << "Error loading " << image_path << ": " << e.what() << endl;
        return false;
    }
}

// ----------------------------------------------------------------------------------------
// Compare two face encodings by computing the Euclidean distance
// ----------------------------------------------------------------------------------------
bool compareFaces(const matrix<float,0,1>& face1, const matrix<float,0,1>& face2, double threshold = 0.6)
{
    double distance = length(face1 - face2);
    cout << "Distance: " << distance << endl;
    return distance < threshold;
}

// ----------------------------------------------------------------------------------------
// Main function
// ----------------------------------------------------------------------------------------
int test()
{
    std::cout << "test1" << std::endl;

    // Initialize dlib's face detector, shape predictor, and face recognition network.
    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor sp;
    // Load the shape predictor model (download from dlib’s website)
    deserialize("shape_predictor_68_face_landmarks.dat") >> sp;

    std::cout<<"loaded shape predictor" <<std::endl;
    anet_type net;
    // Load the face recognition model
    deserialize("mmod_human_face_detector.dat") >> net;
    std::cout<<"loaded recognition model" <<std::endl;


    // Load sample images and compute encodings.
    matrix<float,0,1> daniel_face_descriptor, elon_face_descriptor;
    if (!load_face_encoding("steve.jpg", daniel_face_descriptor, detector, sp, net))
        return 1;
    if (!load_face_encoding("elon.jpeg", elon_face_descriptor, detector, sp, net))
        return 1;

    // Compare the two faces.
    bool match = compareFaces(elon_face_descriptor, daniel_face_descriptor);
    cout << "Faces match: " << (match ? "True" : "False") << endl;

    // Create arrays for known face encodings and their names.
    std::vector<matrix<float,0,1>> known_face_encodings = {daniel_face_descriptor, elon_face_descriptor};
    std::vector<std::string> known_face_names = {"steve", "Elon Musk"};

    // Open video capture using OpenCV.
    cv::VideoCapture video_capture(0);
    if (!video_capture.isOpened()){
        cerr << "Error: Unable to open video capture" << endl;
        return 1;
    }


    return 0;
}
