import { useState, useEffect } from "react";
import { ref, onValue } from "firebase/database";
import { database } from "../firebaseConfig"; // Import Firebase configuration

const SensorData = () => {
  const [sensorData, setSensorData] = useState({});

  useEffect(() => {
    const sensorRef = ref(database, "sensorData"); // Path to your data

    onValue(sensorRef, (snapshot) => {
      if (snapshot.exists()) {
        setSensorData(snapshot.val());
      } else {
        console.log("No data available");
      }
    });

  }, []);

  return (
    <div>
      <h2>Sensor Data</h2>
      <p>Air Temperature: {sensorData.airTemp}</p>
      <p>Distance: {sensorData.distance}</p>
      <p>Humidity: {sensorData.humidity}</p>
      <p>Liquid Temperature: {sensorData.liquidTemp}</p>
      <p>pH Level: {sensorData.pH}</p>
      <p>Total Dissolved Solids (TDS): {sensorData.tds}</p>
    </div>
  );
};

export default SensorData;
