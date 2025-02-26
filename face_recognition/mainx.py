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


# Load sample images and get encodings
steve_face_encoding = load_face_encoding("steve.jpg")
elon_face_encoding = load_face_encoding("elon.jpeg")

# Compare two faces
def compareFaces(face1, face2):
    result = face_recognition.compare_faces([face1], face2)
    print(result[0])
    return result[0]

compareFaces(elon_face_encoding, steve_face_encoding)



# elon = face_recognition.load_image_file("elon.jpeg")
# steve = face_recognition.load_image_file("elon2.jpg")  
# elon_encoding = face_recognition.face_encodings(elon)[0]
# unknown_encoding = face_recognition.face_encodings(steve)[0]
# results = face_recognition.compare_faces([elon_encoding], unknown_encoding)
# print(results)
# if results[0]:
#     print("Success")
# else:
#     print("Failed")









# if elon_face_encoding == elon2_face_encoding:
#     print("True")
# else:
#     print("False")


# # Create arrays of known face encodings and their names (only if encoding exists)
# known_face_encodings = []
# known_face_names = []

# if steve_face_encoding is not None:
#     known_face_encodings.append(steve_face_encoding)
#     known_face_names.append("Steve Jobs")

# if elon_face_encoding is not None:
#     known_face_encodings.append(elon_face_encoding)
#     known_face_names.append("Elon Musk")

# # Ensure there are known faces before running recognition
# if not known_face_encodings:
#     print("Error: No known faces loaded. Exiting...")
#     exit()

# # Get a reference to the webcam
# video_capture = cv2.VideoCapture(0)

# if not video_capture.isOpened():
#     print("Error: Could not open webcam")
#     exit()

# # Initialize some variables
# face_locations = []
# face_encodings = []
# face_names = []
# process_this_frame = True

# while True:
#     # Grab a single frame of video
#     ret, frame = video_capture.read()
#     if not ret:
#         print("Error: Failed to capture frame")
#         break

#     # Process every other frame for performance optimization
#     if process_this_frame:
#         # Resize frame to 1/4 size for faster processing
#         small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
#         rgb_small_frame = small_frame[:, :, ::-1]  # Convert BGR to RGB

#         # Find all face locations in the frame
#         face_locations = face_recognition.face_locations(rgb_small_frame)

#         # Ensure face_locations is not empty before encoding
#         if face_locations:
#             face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)
#         else:
#             face_encodings = []

#         face_names = []
#         for face_encoding in face_encodings:
#             matches = face_recognition.compare_faces(known_face_encodings, face_encoding)
#             name = "Unknown"

#             # Use the face with the smallest distance to the new face
#             face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
#             if len(face_distances) > 0:
#                 best_match_index = np.argmin(face_distances)
#                 if matches[best_match_index]:
#                     name = known_face_names[best_match_index]

#             face_names.append(name)

#     process_this_frame = not process_this_frame

#     # Display results
#     for (top, right, bottom, left), name in zip(face_locations, face_names):
#         top *= 4
#         right *= 4
#         bottom *= 4
#         left *= 4

#         # Draw a rectangle around the face
#         cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
#         cv2.rectangle(frame, (left, bottom - 35), (right, bottom), (0, 0, 255), cv2.FILLED)

#         # Draw label with name
#         font = cv2.FONT_HERSHEY_DUPLEX
#         cv2.putText(frame, name, (left + 6, bottom - 6), font, 1.0, (255, 255, 255), 1)

#     # Display the frame
#     cv2.imshow('Video', frame)

#     # Quit with 'q' key
#     if cv2.waitKey(1) & 0xFF == ord('q'):
#         break

# # Release resources
# video_capture.release()
# cv2.destroyAllWindows()
