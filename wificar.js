const express = require("express");
const bodyParser = require("body-parser");
const app = express();

app.listen(1707, () => {
  console.log("Application started and Listening on port 1707");
});

// server css as static
app.use(express.static(__dirname));

// get our app to use body parser 
app.use(bodyParser.urlencoded({ extended: true }))

app.get("/", (req, res) => {
  res.sendFile(__dirname + "index.html");
});

