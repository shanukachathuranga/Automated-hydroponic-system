import React, { useState, useEffect } from "react";
import {
  Box,
  Typography,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  CardContent,
} from "@mui/material";
import { database } from "../firebaseConfig"; // Adjust the path based on your project
import { ref, set, get } from "firebase/database"; // Import necessary Firebase functions

const Preset = () => {
  const [presets, setPresets] = useState(null); // Store fetched data
  const [open, setOpen] = useState(false);
  const [tempPresets, setTempPresets] = useState(null); // Temporary state for editing

  useEffect(() => {
    const fetchPresets = async () => {
      try {
        const snapshot = await get(ref(database, "presets"));
        if (snapshot.exists()) {
          setPresets(snapshot.val());
        } else {
          console.log("No presets found");
        }
      } catch (error) {
        console.error("Error fetching presets:", error);
      }
    };

    fetchPresets();
  }, []);

  const handleOpen = () => {
    setTempPresets({ ...presets }); // Copy existing presets for editing
    setOpen(true);
  };

  const handleClose = () => setOpen(false);

  const handleSave = async () => {
    if (!tempPresets) return;

    try {
      await set(ref(database, "presets"), tempPresets);
      setPresets(tempPresets);
      console.log("Presets saved successfully!");
    } catch (error) {
      console.error("Error saving presets:", error);
    }

    handleClose();
  };

  const handleChange = (event) => {
    const { name, value } = event.target;
    const parsedValue = value ? parseFloat(value) : 0; // Parse value to number
    setTempPresets({ ...tempPresets, [name]: parsedValue });
  };

  if (!presets) {
    return <Typography>Loading...</Typography>;
  }

  return (
    <CardContent
      sx={{
        height: "100%",
        padding: "20px !important",
        display: "flex",
        flexDirection: "column",
        backgroundColor: "#2C3930",
        color: "white",
        borderRadius: 3,
        gap: 2,
      }}
    >
      <Typography
        variant="h6"
        sx={{
          fontSize: "1.1rem",
          textAlign: "center",
          fontWeight: "bold",
          borderBottom: "2px solid rgba(255,255,255,0.2)",
          pb: 1.5,
          mb: 1,
        }}
      >
        System Presets
      </Typography>

      <Box
        sx={{
          display: "grid",
          gridTemplateColumns: "1fr",
          gap: 1,
          "& .MuiTypography-root": {
            backgroundColor: "rgba(255,255,255,0.1)",
            padding: "10px 14px",
            borderRadius: 1.5,
            fontSize: "0.9rem",
            display: "flex",
            justifyContent: "space-between",
            alignItems: "center",
          },
        }}
      >
        <Typography variant="body2">
          pH Level: <strong>{presets.pH}</strong>
        </Typography>
        <Typography variant="body2">
          TDS Range: <strong>{presets.tdsMin} - {presets.tdsMax} ppm</strong>
        </Typography>
        <Typography variant="body2">
          Pump active Hours: <strong>{presets.pumpStartTime} - {presets.pumpEndTime}</strong>
        </Typography>
        <Typography variant="body2">
          Pump On/Off period: <strong>{presets.pumpOnDuration} / {presets.pumpOffDuration} min</strong>
        </Typography>
        <Typography variant="body2">
          Grow light active Hours: <strong>{presets.lightStartTime} - {presets.lightEndTime}</strong>
        </Typography>
      </Box>

      <Box sx={{ display: "flex", justifyContent: "center", mt: "auto", pt: 0, pb: 1 }}>
        <Button
          variant="contained"
          onClick={handleOpen}
          sx={{
            fontSize: "0.9rem",
            py: 1,
            px: 2,
            minWidth: "160px",
            marginBottom: 5,
            backgroundColor: "#A27B5C",
            color: "white",
            fontWeight: "bold",
            boxShadow: "0 4px 8px rgba(0,0,0,0.2)",
            "&:hover": {
              backgroundColor: "#DCD7C9",
              color: "#A27B5C",
              transform: "translateY(-2px)",
              boxShadow: "0 6px 12px rgba(0,0,0,0.3)",
            },
            transition: "all 0.2s ease-in-out",
          }}
          size="large"
        >
          Modify Presets
        </Button>
      </Box>

      {/* Dialog for modifying presets */}
      <Dialog open={open} onClose={handleClose} PaperProps={{ sx: { borderRadius: 2, minWidth: "320px" } }}>
        <DialogTitle sx={{ pb: 1 }}>Modify System Presets</DialogTitle>
        <DialogContent>
          <Box sx={{ display: "flex", flexDirection: "column", gap: 1.5, pt: 1.5 }}>
            {tempPresets && (
              <>
                <TextField name="pH" label="pH Level" type="number" value={tempPresets.pH} onChange={handleChange} />
                <TextField name="tdsMin" label="Minimum TDS (ppm)" type="number" value={tempPresets.tdsMin} onChange={handleChange} />
                <TextField name="tdsMax" label="Maximum TDS (ppm)" type="number" value={tempPresets.tdsMax} onChange={handleChange} />
                <TextField name="pumpStartTime" label="Pump Start Time" type="number" value={tempPresets.pumpStartTime} onChange={handleChange} />
                <TextField name="pumpEndTime" label="Pump End Time" type="number" value={tempPresets.pumpEndTime} onChange={handleChange} />
                <TextField name="pumpOnDuration" label="Pump On Duration (minutes)" type="number" value={tempPresets.pumpOnDuration} onChange={handleChange} />
                <TextField name="pumpOffDuration" label="Pump Off Duration (minutes)" type="number" value={tempPresets.pumpOffDuration} onChange={handleChange} />
                <TextField name="lightStartTime" label="Light Start Time" type="number" value={tempPresets.lightStartTime} onChange={handleChange} />
                <TextField name="lightEndTime" label="Light End Time" type="number" value={tempPresets.lightEndTime} onChange={handleChange} />
              </>
            )}
          </Box>
        </DialogContent>
        <DialogActions sx={{ p: 2, pt: 1 }}>
          <Button onClick={handleClose}>Cancel</Button>
          <Button onClick={handleSave} variant="contained">
            Save Changes
          </Button>
        </DialogActions>
      </Dialog>
    </CardContent>
  );
};

export default Preset;
