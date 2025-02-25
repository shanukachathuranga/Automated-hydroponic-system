import React from "react";
import { Grid, Card, CardContent, Typography, Box } from "@mui/material";
import { Opacity, WaterDrop, DeviceThermostat, LightMode, Science, Water, AutoGraph } from "@mui/icons-material";
import { useSelector } from "react-redux";

const SensorDataGrid = () => {
  const { sensorData } = useSelector((state) => state.hydroponic);

  const sensorReadings = [
    { label: "pH Level", value: sensorData.pH, icon: <Science fontSize="large" />, color: "#1E88E5" },
    { label: "Water Level (%)", value: `${sensorData.waterLevel} %`, icon: <WaterDrop fontSize="large" />, color: "#43A047" },
    { label: "Humidity (%)", value: `${sensorData.humidity} %`, icon: <Opacity fontSize="large" />, color: "#FF7043" },
    { label: "Grow Light Cycle", value: sensorData.lightCycle, icon: <LightMode fontSize="large" />, color: "#FFD700" },
    { label: "EC Water Quality", value: sensorData.ecQuality, icon: <AutoGraph fontSize="large" />, color: "#8E24AA" },
    { label: "Temperature (°C)", value: `${sensorData.temperature}°C`, icon: <DeviceThermostat fontSize="large" />, color: "#D32F2F" },
    { label: "Water Temp (°C)", value: `${sensorData.waterTemperature}°C`, icon: <Water fontSize="large" />, color: "#1976D2" },
    { label: "Pump Active Cycle", value: sensorData.pumpCycle, icon: <WaterDrop fontSize="large" />, color: "#FF8F00" },
  ];

  return (
    <Grid container spacing={3}>
      {sensorReadings.map((reading, index) => (
        <Grid item xs={12} sm={6} md={3} key={index}>
          <Card
            sx={{
              backgroundColor: reading.color,
              borderRadius: 10,
              boxShadow: "0px 4px 12px rgba(0,0,0,0.2)",
              textAlign: "center",
              color: "white",
              transition: "transform 0.3s ease-in-out",
              "&:hover": { transform: "scale(1.05)" },
            }}
          >
            <CardContent>
              {reading.icon}
              <Typography variant="h6" sx={{ marginTop: 1 }}>
                {reading.label}
              </Typography>
              <Typography variant="h5" sx={{ fontWeight: "bold" }}>
                {reading.value}
              </Typography>
            </CardContent>
          </Card>
        </Grid>
      ))}
    </Grid>
  );
};

export default SensorDataGrid;
