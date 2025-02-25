import React from 'react';
import ReactDOM from 'react-dom';
import { Provider } from 'react-redux';
import App from './App';  // Import your main App component
import store from './redux/store';  // Import your Redux store configuration

// Render the App inside the Provider, making Redux store accessible throughout the app
ReactDOM.render(
  <Provider store={store}>
    <App />
  </Provider>,
  document.getElementById('root')  // This is where your app will be mounted in the DOM
);
