import React, { useEffect, useState } from "react";
import { Line } from "react-chartjs-2";
import { Chart as ChartJS, CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend } from "chart.js";
import { ref, onValue } from "firebase/database";
import { database } from "../firebaseConfig"; // Import Firebase configuration

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend);

const SensorChart = () => {
  const [sensorData, setSensorData] = useState([]);
  const [latestData, setLatestData] = useState({});

  useEffect(() => {
    const sensorRef = ref(database, "sensorData"); // Adjust the path according to your database structure

    // Set up a listener for real-time updates
    const unsubscribe = onValue(sensorRef, (snapshot) => {
      if (snapshot.exists()) {
        const data = snapshot.val();
        setLatestData(data); // This will keep the latest values
      } else {
        console.log("No data available");
      }
    });

    // Clean up listener on component unmount
    return () => unsubscribe();
  }, []);

  // Add the latest data to the chartData every time it updates
  useEffect(() => {
    if (Object.keys(latestData).length > 0) {
      setSensorData((prevData) => {
        const newEntry = {
          ...latestData,
          createdAt: new Date().toLocaleString(), // Adding a timestamp for the x-axis
        };
        return [...prevData, newEntry].slice(-10); // Keep the last 10 entries
      });
    }
  }, [latestData]);

  if (sensorData.length === 0) {
    return <div>Loading sensor data...</div>;
  }

  const chartData = {
    labels: sensorData.map((entry) => entry.createdAt),
    datasets: [
      {
        label: "pH Level",
        data: sensorData.map((entry) => entry.pH),
        borderColor: "#3D8D7A",
        backgroundColor: "#3D8D7A",
        fill: false,
        tension: 0.1,
      },
      {
        label: "Water Level",
        data: sensorData.map((entry) => entry.distance),
        borderColor: "#0288D1",
        backgroundColor: "#0288D1",
        fill: false,
        tension: 0.1,
      },
      {
        label: "Humidity",
        data: sensorData.map((entry) => entry.humidity),
        borderColor: "#71BBB2",
        backgroundColor: "#71BBB2",
        fill: false,
        tension: 0.1,
      },
      {
        label: "Air Temperature",
        data: sensorData.map((entry) => entry.airTemp),
        borderColor: "#F0A04B",
        backgroundColor: "#F0A04B",
        fill: false,
        tension: 0.1,
      },
      {
        label: "Liquid Temp",
        data: sensorData.map((entry) => entry.liquidTemp),
        borderColor: "#789DBC",
        backgroundColor: "#789DBC",
        fill: false,
        tension: 0.1,
      },
    ],
  };

  return (
    <div style={{ display: 'flex', justifyContent: 'space-between' }}>
      <div style={{ width: '48%' }}>
        <h2>Sensor Data Chart</h2>
        <Line data={chartData} />
      </div>
      <div style={{ width: '48%' }}>
        <h2>Total Dissolved Solids (TDS) Chart</h2>
        <TdsChart sensorData={sensorData} />
      </div>
    </div>
  );
};

const TdsChart = ({ sensorData }) => {
  const tdsData = {
    labels: sensorData.map((entry) => entry.createdAt),
    datasets: [
      {
        label: "Total Dissolved Solids (TDS)",
        data: sensorData.map((entry) => entry.tds),
        borderColor: "#8D77AB",
        backgroundColor: "#8D77AB",
        fill: false,
        tension: 0.1,
      },
    ],
  };

  return <Line data={tdsData} />;
};

export default SensorChart;