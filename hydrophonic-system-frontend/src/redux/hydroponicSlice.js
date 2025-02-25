// src/redux/hydroponicSlice.js
import { createSlice, createAsyncThunk } from "@reduxjs/toolkit";

// Async thunk for fetching sensor data
export const fetchSensorData = createAsyncThunk(
  "hydroponic/fetchSensorData",
  async () => {
    const response = await fetch("/api/sensor-data"); // Update with your actual API endpoint
    if (!response.ok) {
      throw new Error("Failed to fetch sensor data");
    }
    const data = await response.json();
    return data;
  }
);

const hydroponicSlice = createSlice({
  name: "hydroponic",
  initialState: {
    sensorData: {},
    status: "idle", // idle | loading | succeeded | failed
    error: null,
  },
  reducers: {
    updateSensorData(state, action) {
      state.sensorData = action.payload;
    },
  },
  extraReducers: (builder) => {
    builder
      .addCase(fetchSensorData.pending, (state) => {
        state.status = "loading";
      })
      .addCase(fetchSensorData.fulfilled, (state, action) => {
        state.status = "succeeded";
        state.sensorData = action.payload;
      })
      .addCase(fetchSensorData.rejected, (state, action) => {
        state.status = "failed";
        state.error = action.error.message;
      });
  },
});

// Export the action to update sensor data if needed
export const { updateSensorData } = hydroponicSlice.actions;

// Export the reducer
export default hydroponicSlice.reducer;
