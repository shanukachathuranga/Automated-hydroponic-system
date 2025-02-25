import React from 'react';
import { Bar } from 'react-chartjs-2';
import { Chart as ChartJS, BarElement, CategoryScale, LinearScale, Title, Tooltip, Legend } from 'chart.js';
import { useSelector } from 'react-redux';

ChartJS.register(BarElement, CategoryScale, LinearScale, Title, Tooltip, Legend);

const LightPumpChart = () => {
  const { sensorData } = useSelector((state) => state.hydroponic);

  const data = {
    labels: ['Grow Light Cycle', 'Water Pump Active Cycle'],
    datasets: [
      {
        label: 'Duration (hours)',
        data: [sensorData.growLightCycle, sensorData.waterPumpCycle],
        backgroundColor: ['#ffbe76', '#6a89cc'],
        borderRadius: 5,
      },
    ],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: { display: false },
      tooltip: { enabled: true },
    },
    scales: {
      y: { beginAtZero: true },
    },
  };

  return (
    <div style={{ height: 200 }}>
      <Bar data={data} options={options} />
    </div>
  );
};

export default LightPumpChart;
