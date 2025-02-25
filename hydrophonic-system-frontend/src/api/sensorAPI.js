// src/api/sensorAPI.js
export const getSensorDataOverTime = async () => {
    try {
      const response = await fetch("http://localhost:5000/api/sensors/over-time");
      const data = await response.json();
      return data;
    } catch (error) {
      console.error("Error fetching sensor data over time:", error);
      throw error;
    }
  };
  