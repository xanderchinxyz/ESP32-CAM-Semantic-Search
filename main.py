from fastapi import FastAPI, File, UploadFile, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse
from pydantic import BaseModel
import socket
from typing import Annotated

from PIL import Image
import io
import torch
import clip

class Text(BaseModel):
    text: str

device = "cuda" if torch.cuda.is_available() else "cpu"
print(device)
model, preprocess = clip.load("ViT-B/32", device=device)

app = FastAPI()

origins = ["*"]

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/calculate_image/")
async def calculate_image(request: Request):
    data: bytes = await request.body()
    image = Image.open(io.BytesIO(data)) #turn bytes to image
    image = preprocess(image).unsqueeze(0).to(device) #preprocess image into tensor
    image_features = model.encode_image(image) #encode image using clip
    vector_list = image_features[0].tolist()
    vector_list = [round(v, 6) for v in vector_list]
    return vector_list

@app.post("/calculate_text/")
async def calculate_text(request: Request):
    text: str = await request.body()
    text = text.decode('utf-8')
    text_tokens = clip.tokenize(text)
    text_features = model.encode_text(text_tokens)
        
    return text_features[0].tolist()

@app.get("/")
async def root():
    return {"message: hello world"}


hostname = socket.gethostname()
IPAddr = socket.gethostbyname(hostname)

print("Your Computer Name is:" + hostname)
print("Access the server by going to: http://" + IPAddr + ":8000")