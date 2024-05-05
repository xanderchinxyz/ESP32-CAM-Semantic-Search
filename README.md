# ESP32-CAM-Semantic-Search
Essentially:
1. ESP32-CAM takes a picture every 10 seconds
2. The picture is stored on the mounted SD card with the current local epoch time as the title
3. The picture is also sent to the backend server and its vector embedding is generated using OpenAI CLIP
4. This vector embedding corresponding picture title is then appended to the vectors.bin file
5. When the website hosted on the ESP32-CAM is opened the user can enter a text description
6. The text description is sent to the backend server and its vector embedding is generated using OpenAI CLIP
7. The text vector and all picture vectors are compared using the cosine similarity function
8. Pictures are shown and sorted by the highest cosine similarity (aka pictures that match the text description the most)
9. Pictures can also be filtered by time

# Installation Instructions:
This will come later cause the whole project is still a work in progress

# Current Issues and Limitations (and Potential Fixes?):
- Images sometimes take a long time to load due to the SD card having to open multiple image files. Currently, the short-term solution is to press the search button again if an image doesn't load properly. The long-term solution is to maybe use an MJPEG file to store all images similar to how all the vectors are stored in one binary file
- Blurry and dim photos from the ESP32-CAM if the user is moving around or the lighting is bad, which kind of confuses the CLIP model. The solution is to play around with the camera settings or just get a better camera (the ESP32-CAM was CAD 10)
- The CLIP model is general and not optimized to the user's unique objects so it probably can't pick out a very specific cup or object you have
- If the picture has too many objects in it the semantic search for a specific object may not work well
