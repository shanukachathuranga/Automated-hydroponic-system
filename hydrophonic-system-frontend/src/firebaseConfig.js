import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

const firebaseConfig = {
  apiKey: "ADD API KEY HERE",
  authDomain: "hydroponic-system-fa9cc.firebaseapp.com",
  databaseURL: "ADD DB URL HERE",
  projectId: "hydroponic-system-fa9cc",
  storageBucket: "hydroponic-system-fa9cc.firebasestorage.app",
  messagingSenderId: "188274114697",
  appId: "1:188274114697:web:5866da0fccee4d7089c658"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

export { database };
