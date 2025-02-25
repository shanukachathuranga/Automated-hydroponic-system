import React, { useState, useEffect, useRef } from "react";
import { Grid, Card, CardContent, Typography, Switch, Box } from "@mui/material";
import { EvStation, Science, LightMode, ElectricBolt, WaterDrop } from "@mui/icons-material";
import { getDatabase, ref, onValue, set } from "firebase/database"; // Import necessary functions

const ControlPanel = () => {
  const [controlState, setControlState] = useState({
    waterMotor: false,
    phMotorUp: false,
    phMotorDown: false,
    growLight: false,
    fertilizer: false,
    solenoid: false,
  });

  const [sensorData, setSensorData] = useState({ ph: 0, ppm: 0, temperature: 0, waterLevel: 0 });
  const prevState = useRef({ phMotorUp: false, phMotorDown: false, fertilizer: false });

  // Firebase database reference
  const db = getDatabase();

  // Listen for updates from Firebase
  useEffect(() => {
    const dbRef = ref(db, 'systemControl');

    // Listen for updates
    const listener = onValue(dbRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        setControlState(data);
      }
    });

    return () => {
      // Unsubscribe from listener on component unmount
      listener();
    };
  }, [db]);

  const toggleControl = async (key, state) => {
    try {
      // Update control state in Firebase
      const newState = { ...controlState, [key]: state !== undefined ? state : !controlState[key] };
      await set(ref(db, 'systemControl'), newState);
      setControlState(newState);
    } catch (error) {
      console.error("Error toggling control:", error);
    }
  };

  // Automatic controls based on sensor data
  useEffect(() => {
    if (sensorData.ph > 9 && !prevState.current.phMotorDown) {
      toggleControl("phMotorDown", true);
      prevState.current.phMotorDown = true;
      prevState.current.phMotorUp = false;
    } else if (sensorData.ph < 7 && !prevState.current.phMotorUp) {
      toggleControl("phMotorUp", true);
      prevState.current.phMotorUp = true;
      prevState.current.phMotorDown = false;
    }

    if (sensorData.ppm < 300 && !prevState.current.fertilizer) {
      toggleControl("fertilizer", true);
      prevState.current.fertilizer = true;
    }
  }, [sensorData]);

  const controlItems = [
    { key: "waterMotor", label: "Water Motor", icon: <EvStation fontSize="large" />, color: "#0766AD" },
    { key: "phMotorUp", label: "pH Up", icon: <Science fontSize="large" />, color: "#36AE7C" },
    { key: "phMotorDown", label: "pH Down", icon: <Science fontSize="large" />, color: "#BC7AF9" },
    { key: "growLight", label: "Grow Light", icon: <LightMode fontSize="large" />, color: "#FAA300" },
    { key: "fertilizer", label: "Fertilizer", icon: <ElectricBolt fontSize="large" />, color: "#DD4A48" },
    { key: "solenoid", label: "Solenoid", icon: <WaterDrop fontSize="large" />, color: "#279EFF" },
  ];

  return (
    <Box sx={{ marginTop: 3 }}>
      <Typography variant="h5" sx={{ marginBottom: 2 }}>
        System Controls
      </Typography>
      <Grid container spacing={3}>
        {controlItems.map((item) => (
          <Grid item xs={12} sm={6} md={4} key={item.key}>
            <Card
              sx={{
                backgroundColor: item.color,
                borderRadius: 2,
                boxShadow: "0px 4px 12px rgba(0,0,0,0.2)",
                textAlign: "center",
                color: "white",
              }}
            >
              <CardContent>
                {item.icon}
                <Typography variant="h6">{item.label}</Typography>
                <Switch
                  checked={controlState[item.key]}
                  onChange={() => toggleControl(item.key)}
                  color="default"
                  sx={{
                    "& .MuiSwitch-switchBase.Mui-checked": { color: "white" },
                    "& .MuiSwitch-switchBase.Mui-checked + .MuiSwitch-track": { backgroundColor: "white" },
                  }}
                />
                <Typography variant="h6" sx={{ fontWeight: "bold", marginTop: 1 }}>
                  {controlState[item.key] ? "ON / AUTO" : "OFF"}
                </Typography>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>
    </Box>
  );
};

export default ControlPanel;
