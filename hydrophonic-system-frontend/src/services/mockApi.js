const mockSensorData = [
  { pH: 6.8, waterLevel: 45, humidity: 55 },
  { pH: 7.0, waterLevel: 50, humidity: 60 },
  { pH: 7.2, waterLevel: 53, humidity: 62 },
];

export const fetchMockSensorData = () => {
  return new Promise((resolve) => {
    setTimeout(() => {
      const randomData = mockSensorData[Math.floor(Math.random() * mockSensorData.length)];
      resolve(randomData);
    }, 2000); // Simulate API delay
  });
};
