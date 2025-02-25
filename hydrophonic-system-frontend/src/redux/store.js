// src/redux/store.js
import { configureStore } from "@reduxjs/toolkit";
import hydroponicReducer from "./hydroponicSlice";

const store = configureStore({
  reducer: {
    hydroponic: hydroponicReducer,
  },
});

export default store;
