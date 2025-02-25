import React, { useEffect, useState } from "react";
import { ref, onValue } from "firebase/database";
import { database } from "../firebaseConfig";
import { Box, Grid, Card, Typography } from "@mui/material";
import { CheckCircle, Cancel, EvStation, Science, LightMode, ElectricBolt, WaterDrop } from "@mui/icons-material";

// Define system activity details with icons and colors
const activityDetails = [
  { key: "waterMotor", label: "Water Motor", icon: <EvStation fontSize="large" />, color: "#0766AD" },
  { key: "phMotorUp", label: "pH Up", icon: <Science fontSize="large" />, color: "#36AE7C" },
  { key: "phMotorDown", label: "pH Down", icon: <Science fontSize="large" />, color: "#BC7AF9" },
  { key: "growLight", label: "Grow Light", icon: <LightMode fontSize="large" />, color: "#FAA300" },
  { key: "fertilizer", label: "Fertilizer", icon: <ElectricBolt fontSize="large" />, color: "#DD4A48" },
  { key: "solenoid", label: "Solenoid", icon: <WaterDrop fontSize="large" />, color: "#279EFF" },
];

const SystemActivityGrid = () => {
  const [systemActivity, setSystemActivity] = useState({});

  useEffect(() => {
    const activityRef = ref(database, "systemActivity");

    const unsubscribe = onValue(activityRef, (snapshot) => {
      if (snapshot.exists()) {
        setSystemActivity(snapshot.val());
      } else {
        console.log("No system activity data available");
      }
    });

    return () => unsubscribe();
  }, []);

  return (
    <Box sx={{ mt: 3, width: "100%" }}>
      <Typography
        variant="h5"
        sx={{
          mb: 3,
          fontWeight: 600,
          textAlign: "center",
          color: "#37474F",
        }}
      >
        System Activity Status
      </Typography>
      <Grid container spacing={3}>
        {activityDetails.map(({ key, label, icon, color }) => {
          const isActive = systemActivity[key] ?? false;
          return (
            <Grid item xs={12} sm={6} md={4} key={key}>
              <Card
                sx={{
                  display: "flex",
                  alignItems: "center",
                  justifyContent: "space-between",
                  padding: 3,
                  borderRadius: 3,
                  boxShadow: "0px 4px 15px rgba(0,0,0,0.1)",
                  background: isActive
                    ? "linear-gradient(135deg, #C8E6C9, #A5D6A7)"
                    : "linear-gradient(135deg, #FFCDD2, #EF9A9A)",
                  transition: "transform 0.3s ease-in-out",
                  "&:hover": { transform: "scale(1.05)" },
                }}
              >
                <Box sx={{ display: "flex", alignItems: "center", gap: 2 }}>
                  <Box
                    sx={{
                      width: 50,
                      height: 50,
                      display: "flex",
                      alignItems: "center",
                      justifyContent: "center",
                      borderRadius: "50%",
                      backgroundColor: color,
                      color: "#FFF",
                    }}
                  >
                    {icon}
                  </Box>
                  <Typography
                    sx={{
                      fontSize: "1.4rem",
                      fontWeight: 500,
                      textTransform: "capitalize",
                      color: isActive ? "#1B5E20" : "#B71C1C",
                    }}
                  >
                    {label}
                  </Typography>
                </Box>
                <Box sx={{ display: "flex", alignItems: "center" }}>
                  {isActive ? (
                    <>
                      <CheckCircle sx={{ color: "#00796B", fontSize: "2rem" }} />
                      <Typography sx={{ marginLeft: 1, color: "#00796B" }}>Active</Typography>
                    </>
                  ) : (
                    <>
                      <Cancel sx={{ color: "#D32F2F", fontSize: "2rem" }} />
                      <Typography sx={{ marginLeft: 1, color: "#D32F2F" }}>Inactive</Typography>
                    </>
                  )}
                </Box>
              </Card>
            </Grid>
          );
        })}
      </Grid>
    </Box>
  );
};

export default SystemActivityGrid;
