// TdsChart.js
import React, { useEffect, useState } from "react";
import { Line } from "react-chartjs-2";
import { ref, onValue } from "firebase/database";
import { database } from "../firebaseConfig"; // Import Firebase configuration
import { Chart as ChartJS, CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend } from "chart.js";

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend);

const TdsChart = () => {
  const [sensorData, setSensorData] = useState([]);
  const [latestData, setLatestData] = useState({});

  useEffect(() => {
    const sensorRef = ref(database, "sensorData");

    const unsubscribe = onValue(sensorRef, (snapshot) => {
      if (snapshot.exists()) {
        const data = snapshot.val();
        setLatestData(data);
      } else {
        console.log("No data available");
      }
    });

    return () => unsubscribe();
  }, []);

  useEffect(() => {
    if (Object.keys(latestData).length > 0) {
      setSensorData((prevData) => {
        const newEntry = {
          ...latestData,
          createdAt: new Date().toLocaleString(),
        };
        return [...prevData, newEntry].slice(-10);
      });
    }
  }, [latestData]);

  if (sensorData.length === 0) {
    return <div>Loading sensor data...</div>;
  }

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

  return (
    <div style={{ width: '100%' }}>
      <h2>Total Dissolved Solids (TDS) Chart</h2>
      <Line data={tdsData} />
    </div>
  );
};

export default TdsChart;
