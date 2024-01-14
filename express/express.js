const express = require('express');
const bodyParser = require('body-parser');
const axios = require('axios');
const cors = require('cors');

const app = express();
const port = 3000; // Choose the desired port for your Express server
app.use(cors());
// Parse JSON bodies
app.use(bodyParser.json());

// Endpoint for GET request
app.get('/get/:month', async (req, res) => {
  try {
    const month = req.params.month;
    const response = await axios.get(`http://localhost:8080/get/${month}`);
    res.json(response.data);
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});

// Endpoint for POST request
app.post('/insert/:month', async (req, res) => {
  try {
    const month = req.params.month;
    const response = await axios.post(`http://localhost:8080/insert/${month}`, req.body);
    res.json(response.data);
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});

// Endpoint for PUT request
app.put('/update/:month', async (req, res) => {
  try {
    const month = req.params.month;
    const response = await axios.put(`http://localhost:8080/update/${month}`, req.body);
    res.json(response.data);
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});

// Endpoint for DELETE request
app.delete('/delete/:month', async (req, res) => {
  try {
    const month = req.params.month;
    const response = await axios.delete(`http://localhost:8080/delete/${month}`, { data: req.body });
    res.json(response.data);
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});

// Start the Express server
app.listen(port, () => {
  console.log(`Express server is running on http://localhost:${port}`);
});
