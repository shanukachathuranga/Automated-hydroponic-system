const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors'); // If needed for cross-origin requests
const sensorRoutes = require('./routes/sensorRoutes'); // Import the routes file

const app = express();
const PORT = 5000;

// Middleware
app.use(cors()); // Enable CORS (if needed)
app.use(express.json()); // Parse JSON bodies in requests

// MongoDB connection
mongoose.connect('mongodb://localhost/hydroponic', {
  useNewUrlParser: true,
  useUnifiedTopology: true,
})
.then(() => console.log('MongoDB connected'))
.catch((err) => console.log(err));

// Use the sensor routes for any requests starting with /api/sensors
app.use('/api/sensors', sensorRoutes);

// Start the server
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));
