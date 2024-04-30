# ESP32-CAM-Semantic-Search
Essentially:
1. ESP32-CAM takes picture every 10 seconds
2. Stores it on the mounted SD card
3. Picture is sent to the backend server and its vector embedding is generated using OpenAI CLIP
4. When website is opened user can enter a text description
5. Text description is sent to the backend server and its vector embedding is generated usin OpenAI CLIP
6. Text vector and all picture vectors are compared using cosine similarity
7. Pictures are shown and sorted by the highest cosine similarity (aka pictures that match the text description the most)

# Installation Instructions:
This will come later cause the whole project is still a work in progress
