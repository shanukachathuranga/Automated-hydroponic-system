import React from 'react';
import { Box, Typography } from '@mui/material';
import Dashboard from './components/Dashboard';
import { LocalFlorist } from "@mui/icons-material";
function App() {
  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        minHeight: '100vh',
      }}
    >
      
      {/* Header Section */}
      <Box
      
        sx={{
          backgroundColor: '#118B50',
          color: 'white',
          padding: '16px',
          textAlign: 'center',
        }}
      >
        <LocalFlorist sx={{ marginRight: 2, fontSize: '3rem' }} /> {/* Icon with some right margin */}
<Typography 
  variant="h4" 
  sx={{ 
    fontWeight: 'bold', 
    fontFamily: 'Arial, sans-serif',
    color: '#FFFFFF',
    textAlign: 'center', 
    display: 'flex', 
    alignItems: 'center', 
    justifyContent: 'center' 
  }}
>
  
  Smart Hydroponic System
</Typography>

      </Box>

      {/* Main Content Section */}
      <Box
        sx={{
          display: 'flex',
          flex: 1,
          padding: '20px',
          gap: 2,
        }}
      >
        <Dashboard />
      </Box>

      {/* Footer Section */}
      <Box
        sx={{
          backgroundColor: '#3f51b5',
          color: 'white',
          padding: '16px',
          textAlign: 'center',
        }}
      >
        <Typography variant="body1">&copy; 2025 Hydroponic System</Typography>
      </Box>
    </Box>
  );
}

export default App;
