var picDict = [];
async function loadFromSDCard() {
    //read vector list off of binary file
    const fileName = await fetch('./vectors.bin')
    const data = await fileName.blob()
    
    var reader = new FileReader();
    reader.onload = function(event) {
        var buffer = event.target.result;       // Get the ArrayBuffer containing the blob data
        var dataView = new DataView(buffer);    // Create a DataView to interpret the buffer as binary data
        var floatTest = new Float32Array(buffer);
        var numPhotos = dataView.byteLength/2052;

        for(let x = 0; x < numPhotos; x++) {
            var first32Bits = dataView.getInt32(x*2052, true); // Read the first 32 bits (4 bytes) from the DataView (true for little-endian)
            var floatList = floatTest.subarray(1*(x+1)+512*x, 1*(x+1)+512*x+512)
            picDict.push({
                fileName: String(first32Bits),
                vector: floatList,   //skip first 4 bytes as that is the file name
            })
        }
        document.getElementById('loading-status').textContent = 'Loaded ' + numPhotos + ' Photos'
    }
    reader.readAsArrayBuffer(data);

    //send time to esp32-cam
    var now = new Date();
    var rtcTime = now.toLocaleTimeString('en-US', {hour12: false}).split(":")
    rtcTime[2] = parseInt(rtcTime[2])
    var rtcDate = now.toLocaleDateString().split("/")
    console.log(rtcDate);
    fetch(window.location.href + "send-time", {
        method: "POST",
        headers: {
        'Content-Type': 'text/plain',
        },
        body: rtcTime[2]+","+rtcTime[1]+","+rtcTime[0]+","+rtcDate[1]+","+rtcDate[0]+","+rtcDate[2],
    })
    .then(response => response.text()) // Get the response as text
    .then(textData => {

    })
}

async function calculateSimilarity(textToSend, numPhotos) {
    try {
        //get text vector
        let serverResponse = await fetch('http://192.168.1.139:8000/calculate_text/', {
            method: 'POST',
            body: String(textToSend),
        });
        let textVector = await serverResponse.json();
        for(let x = 0; x < picDict.length; x++) {
            picDict[x].similarity = cosineSimilarity(textVector, picDict[x].vector)
        }

        // sort by similarity in descending 
        picDict.sort((a, b) => {
            return b.similarity - a.similarity;
        })

        // display images
        const imageContainer = document.getElementById('image-container');
        imageContainer.innerHTML = "";
        picDict.slice(0, numPhotos).forEach(picture => {
            const container = document.createElement('div');

            const image = document.createElement('img');
            image.src = 'Pictures/' + picture.fileName + '.jpg'; // Assuming the images are stored in an "images" folder
            image.alt = picture.fileName + '.jpg';
            container.appendChild(image);

            const time = document.createElement('p');
            console.log(parseInt(picture.fileName))
            var date = new Date(0);
            date.setSeconds(parseInt(picture.fileName) + date.getTimezoneOffset()*60);
            time.innerText = date.toLocaleString();
            container.appendChild(time)

            const similarity = document.createElement('p');
            similarity.innerText = "Similarity: " + picture.similarity;
            container.appendChild(similarity)

            imageContainer.appendChild(container)
        });
    } catch (error) {
        console.error('Error:', error);
    }
}

async function fetchAllPhotos() {
    // sort by most recent
    picDict.sort((a, b) => {
        return b.fileName - a.fileName
    })

    // display images
    const imageContainer = document.getElementById('image-container');
    imageContainer.innerHTML = "";
    picDict.forEach(picDict => {
        const image = document.createElement('img');
        image.src = 'Pictures/' + picDict.fileName + '.jpg'; // Assuming the images are stored in an "images" folder
        image.alt = picDict.fileName + '.jpg';
        imageContainer.appendChild(image);
    });
}

function changeToggle(id, text1, text2) {
    element = document.getElementById(id)
    if(element.innerText == text2) {
        element.innerText = text1
    } else {
        element.innerText = text2
    }
}

function toggleCapture() {
    element = document.getElementById('capture-status')
    if(element.innerText == "Capture Off") {
        element.innerText = "Capture On"
        sendPOST('capture-on')
    } else {
        element.innerText = "Capture Off"
        sendPOST('capture-off')
    }
}

function cosineSimilarity(vec1, vec2) {
    const dot = dotProduct(vec1, vec2);
    const mag1 = magnitude(vec1);
    const mag2 = magnitude(vec2);

    // Handle division by zero
    if (mag1 === 0 || mag2 === 0) {
        return 0;
    }

    return dot / (mag1 * mag2);
}

function sendPOST(action) {
    var url = window.location.href + action;
    fetch(url, {
        method: "POST",
        headers: {
        'Content-Type': 'text/plain',
        },
        body: String(action),
    })
    .then(response => response.text()) // Get the response as text
    .then(textData => {
        // Handle the data from the response
        //console.log('Response Data:', textData);
    })
}

function dotProduct(vec1, vec2) {
    if (vec1.length !== vec2.length) {
        throw new Error("Vectors must have the same length.");
    }
    
    let product = 0;
    for (let i = 0; i < vec1.length; i++) {
        product += vec1[i] * vec2[i];
    }
    return product;
}

// Function to compute the magnitude (Euclidean norm) of a vector
function magnitude(vec) {
    let sumOfSquares = 0;
    for (let i = 0; i < vec.length; i++) {
        sumOfSquares += vec[i] * vec[i];
    }
    return Math.sqrt(sumOfSquares);
}