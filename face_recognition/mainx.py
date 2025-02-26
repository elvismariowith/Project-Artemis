import face_recognition
import cv2
import numpy as np
import os


# Function to load an image and get its face encoding
def load_face_encoding(image_path):
    # Make sure the image exists
    if not os.path.exists(image_path):
        print(f"Error: {image_path} not found!")
        return None
    
    # Convert image to an array RGB Values to more easily detect the face(s) in the picture
    image = face_recognition.load_image_file(image_path)
    
    # Pass in the RGB image to the face_recognition library to get the face encoding
    encodings = face_recognition.face_encodings(image)

    if len(encodings) > 0:
        return encodings[0]
    else:
        print(f"No face found in {image_path}")
        return None

# Function to compare two faces
def compareFaces(face1, face2):
    result = face_recognition.compare_faces([face1], face2)
    print(result[0])
    return result[0]

# Load sample images and get encodings
steve_face_encoding = load_face_encoding("steve.jpg")
elon_face_encoding = load_face_encoding("elon.jpeg")


compareFaces(elon_face_encoding, steve_face_encoding)


# Create arrays of known face encodings and their names
known_face_encodings = []
known_face_names = []

# Add to the arrays
known_face_encodings.append(steve_face_encoding)
known_face_names.append("Steve Jobs")

known_face_encodings.append(elon_face_encoding)
known_face_names.append("Elon Musk")

# Initialize a video capture object
video_capture = cv2.VideoCapture(0)



# Current Face Information
detected_faces = []
face_encodings = []
face_names = []

# Process every other frame for performance optimization
process_this_frame = True

while True:
    # ret is a boolean that returns true if the frame is available and frame is the actual frame
    ret, frame = video_capture.read()
    if not ret:
        print("Error: Failed to capture frame")
        break

    if process_this_frame:

        # Resize frame to 1/4 size for faster processing
        small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)

        # Convert BGR to RGB for face_recognition library
        rgb_small_frame = small_frame[:, :, ::-1]  # Convert BGR to RGB

        # Detects all the faces in the frame. It returns a list of coordinates of the detected faces
        detected_faces = face_recognition.face_locations(rgb_small_frame)

        # Get the face encodings for all the detected faces
        if detected_faces:
            # TODO: Check arguments for face_encodings
            face_encodings = face_recognition.face_encodings(rgb_small_frame, detected_faces)
        else:
            face_encodings = []

        face_names = []


        for i in face_encodings:
            matches = face_recognition.compare_faces(known_face_encodings, i)
            name = "Unknown"

            # Use the face with the smallest distance to the new face
            face_distances = face_recognition.face_distance(known_face_encodings, i)
            if len(face_distances) > 0:
                best_match_index = np.argmin(face_distances)
                if matches[best_match_index]:
                    name = known_face_names[best_match_index]

            face_names.append(name)

    process_this_frame = not process_this_frame

    # Display results
    for name in face_names:
        print(f"Face found: {name}")

    # Display the frame
    cv2.imshow('Video', frame)

    # Quit with 'q' key
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release resources
video_capture.release()
cv2.destroyAllWindows()
