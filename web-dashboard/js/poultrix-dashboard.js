// Poultrix Edge AI Dashboard - Firebase Integration
// Competition: Edge Impulse Innovation Challenge 2025

// Firebase configuration
const firebaseConfig = {
    databaseURL: "YOUR_FIREBASE_DATABASE_URL_HERE"
};

// Global variables
let temperatureChart, humidityChart, gasChart;
let isConnected = false;
let app, database;
let aiInsightsEnabled = true;

// Initialize Firebase and dashboard when page loads
window.addEventListener('DOMContentLoaded', async function() {
    console.log('🚀 Poultrix Edge AI Dashboard Starting...');
    
    // Import Firebase modules
    const { initializeApp } = await import('https://www.gstatic.com/firebasejs/10.5.0/firebase-app.js');
    const { getDatabase, ref, onValue, off } = await import('https://www.gstatic.com/firebasejs/10.5.0/firebase-database.js');
    
    // Initialize Firebase
    app = initializeApp(firebaseConfig);
    database = getDatabase(app);
    
    console.log('✅ Firebase initialized for Edge AI system');
    
    // Initialize charts and start data fetching
    initializeCharts();
    startDataFetching();
    initializeAIInsights();
});

// Initialize enhanced charts for competition
function initializeCharts() {
    // Temperature Chart with enhanced styling
    const tempCtx = document.getElementById('temperatureChart').getContext('2d');
    temperatureChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Temperature (°C)',
                data: [],
                borderColor: '#ff9a3c',
                backgroundColor: 'rgba(255, 154, 60, 0.2)',
                tension: 0.4,
                fill: true,
                borderWidth: 3,
                pointRadius: 5,
                pointHoverRadius: 8
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                title: {
                    display: true,
                    text: 'Smart Temperature Control',
                    font: {
                        size: 16,
                        weight: 'bold'
                    }
                },
                legend: {
                    display: true,
                    position: 'top'
                }
            },
            scales: {
                y: {
                    beginAtZero: false,
                    min: 20,
                    max: 40,
                    title: {
                        display: true,
                        text: 'Temperature (°C)',
                        font: {
                            weight: 'bold'
                        }
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.1)'
                    }
                },
                x: {
                    title: {
                        display: true,
                        text: 'Time',
                        font: {
                            weight: 'bold'
                        }
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.1)'
                    }
                }
            },
            interaction: {
                intersect: false,
                mode: 'index'
            }
        }
    });
    
    // Humidity Chart with enhanced styling
    const humidCtx = document.getElementById('humidityChart').getContext('2d');
    humidityChart = new Chart(humidCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Humidity (%)',
                data: [],
                borderColor: '#ffcd3c',
                backgroundColor: 'rgba(255, 205, 60, 0.2)',
                tension: 0.4,
                fill: true,
                borderWidth: 3,
                pointRadius: 5,
                pointHoverRadius: 8
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                title: {
                    display: true,
                    text: 'Humidity Monitoring',
                    font: {
                        size: 16,
                        weight: 'bold'
                    }
                },
                legend: {
                    display: true,
                    position: 'top'
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    max: 100,
                    title: {
                        display: true,
                        text: 'Humidity (%)',
                        font: {
                            weight: 'bold'
                        }
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.1)'
                    }
                },
                x: {
                    title: {
                        display: true,
                        text: 'Time',
                        font: {
                            weight: 'bold'
                        }
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.1)'
                    }
                }
            },
            interaction: {
                intersect: false,
                mode: 'index'
            }
        }
    });
    
    // Gas Level Chart with enhanced styling
    const gasCtx = document.getElementById('gasChart').getContext('2d');
    gasChart = new Chart(gasCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Air Quality (ppm)',
                data: [],
                borderColor: '#28a745',
                backgroundColor: 'rgba(40, 167, 69, 0.2)',
                tension: 0.4,
                fill: true,
                borderWidth: 3,
                pointRadius: 5,
                pointHoverRadius: 8
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                title: {
                    display: true,
                    text: 'AI-Powered Air Quality',
                    font: {
                        size: 16,
                        weight: 'bold'
                    }
                },
                legend: {
                    display: true,
                    position: 'top'
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    title: {
                        display: true,
                        text: 'Gas Level (ppm)',
                        font: {
                            weight: 'bold'
                        }
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.1)'
                    }
                },
                x: {
                    title: {
                        display: true,
                        text: 'Time',
                        font: {
                            weight: 'bold'
                        }
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.1)'
                    }
                }
            },
            interaction: {
                intersect: false,
                mode: 'index'
            }
        }
    });

    console.log('📊 Enhanced charts initialized for competition dashboard');
}

// Start fetching data from Firebase
async function startDataFetching() {
    const { ref, onValue } = await import('https://www.gstatic.com/firebasejs/10.5.0/firebase-database.js');
    const deviceRef = ref(database, 'devices/poultrix_esp_001/latest');
    
    onValue(deviceRef, (snapshot) => {
        if (snapshot.exists()) {
            const data = snapshot.val();
            updateDashboard(data);
            updateConnectionStatus(true);
            processAIInsights(data);
        } else {
            updateConnectionStatus(false);
            console.log("No data available");
        }
    }, (error) => {
        updateConnectionStatus(false);
        console.error("Firebase error:", error);
    });

    console.log('🔄 Real-time data fetching started');
}

// Initialize AI insights system
function initializeAIInsights() {
    console.log('🤖 AI Insights system initialized');
    // Placeholder for Edge Impulse ML model integration
    // This would be where you integrate your trained models
}

// Process AI insights from sensor data
function processAIInsights(data) {
    if (!aiInsightsEnabled) return;

    // Simulate AI-powered insights
    // In a real implementation, this would use Edge Impulse models
    
    // Example: Anomaly detection
    if (data.temperature && data.humidity && data.gasLevel) {
        const tempAnomaly = Math.abs(data.temperature - 28) > 5;
        const humidityAnomaly = data.humidity > 80 || data.humidity < 40;
        const gasAnomaly = data.gasLevel > 500;

        if (tempAnomaly || humidityAnomaly || gasAnomaly) {
            console.log('⚠️ AI Alert: Environmental anomaly detected');
            // Here you would trigger notifications or alerts
        }
    }
}

// Update dashboard with new data
function updateDashboard(data) {
    // Update current values with enhanced formatting
    updateElement('current-temp', data.temperature ? data.temperature.toFixed(1) + '°C' : 'N/A');
    updateElement('current-humidity', data.humidity ? data.humidity.toFixed(0) + '%' : 'N/A');
    updateElement('current-gas', data.gasLevel ? data.gasLevel.toFixed(1) + ' ppm' : 'N/A');
    updateElement('current-setpoint', data.setpoint ? data.setpoint.toFixed(1) + '°C' : 'N/A');
    updateElement('current-pid', data.pidOutput || 0);
    updateElement('free-heap', data.freeHeap ? (data.freeHeap / 1024).toFixed(1) + ' KB' : 'N/A');
    updateElement('wifi-signal', data.wifiSignal ? data.wifiSignal + ' dBm' : 'N/A');
    updateElement('uptime', data.uptime ? formatUptime(data.uptime) : 'N/A');
    
    // Update status indicators with enhanced styling
    updateStatus('heater-status', data.heaterActive);
    updateStatus('fan-status', data.fanActive);
    updateStatus('temp-valid', data.tempValid);
    updateStatus('humidity-valid', data.humidityValid);
    updateStatus('gas-valid', data.gasValid);
    
    // Update last update time
    updateElement('last-update', new Date().toLocaleTimeString());
    
    // Update charts with new data
    updateCharts(data);
    
    // Update temperature card color based on AI analysis
    updateTemperatureCard(data.temperature, data.setpoint);
}

// Update individual elements
function updateElement(id, value) {
    const element = document.getElementById(id);
    if (element) {
        element.textContent = value;
        // Add loading animation removal
        element.classList.remove('loading');
    }
}

// Update status indicators with enhanced styling
function updateStatus(id, isActive) {
    const element = document.getElementById(id);
    if (element) {
        if (isActive) {
            element.className = 'badge bg-success';
            element.textContent = 'Active';
        } else {
            element.className = 'badge bg-secondary';
            element.textContent = 'Inactive';
        }
        element.classList.remove('loading');
    }
}

// Update connection status with enhanced feedback
function updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connection-status');
    const statusIcon = document.getElementById('status-icon');
    
    if (connected) {
        statusElement.className = 'badge bg-success';
        statusElement.textContent = 'Connected';
        statusIcon.className = 'fas fa-wifi text-success';
        isConnected = true;
        console.log('✅ Connected to Poultrix Edge AI system');
    } else {
        statusElement.className = 'badge bg-danger';
        statusElement.textContent = 'Disconnected';
        statusIcon.className = 'fas fa-wifi-slash text-danger';
        isConnected = false;
        console.log('❌ Disconnected from Poultrix Edge AI system');
    }
}

// Update charts with enhanced data handling
function updateCharts(data) {
    const currentTime = new Date().toLocaleTimeString();
    
    // Update temperature chart
    if (data.temperature) {
        temperatureChart.data.labels.push(currentTime);
        temperatureChart.data.datasets[0].data.push(data.temperature);
        
        // Keep only last 25 points for better performance
        if (temperatureChart.data.labels.length > 25) {
            temperatureChart.data.labels.shift();
            temperatureChart.data.datasets[0].data.shift();
        }
        
        temperatureChart.update('none');
    }
    
    // Update humidity chart
    if (data.humidity) {
        humidityChart.data.labels.push(currentTime);
        humidityChart.data.datasets[0].data.push(data.humidity);
        
        // Keep only last 25 points
        if (humidityChart.data.labels.length > 25) {
            humidityChart.data.labels.shift();
            humidityChart.data.datasets[0].data.shift();
        }
        
        humidityChart.update('none');
    }
    
    // Update gas chart
    if (data.gasLevel) {
        gasChart.data.labels.push(currentTime);
        gasChart.data.datasets[0].data.push(data.gasLevel);
        
        // Keep only last 25 points
        if (gasChart.data.labels.length > 25) {
            gasChart.data.labels.shift();
            gasChart.data.datasets[0].data.shift();
        }
        
        gasChart.update('none');
    }
}

// Update temperature card with AI-enhanced color coding
function updateTemperatureCard(temperature, setpoint) {
    const tempCard = document.querySelector('.temperature-card');
    if (tempCard && temperature && setpoint) {
        const diff = Math.abs(temperature - setpoint);
        
        if (diff <= 1.5) {
            tempCard.className = 'card temperature-card border-success';
            tempCard.style.borderColor = '#28a745';
            tempCard.style.boxShadow = '0 4px 15px rgba(40, 167, 69, 0.3)';
        } else if (diff <= 3) {
            tempCard.className = 'card temperature-card border-warning';
            tempCard.style.borderColor = '#ff9a3c';
            tempCard.style.boxShadow = '0 4px 15px rgba(255, 154, 60, 0.3)';
        } else {
            tempCard.className = 'card temperature-card border-danger';
            tempCard.style.borderColor = '#dc3545';
            tempCard.style.boxShadow = '0 4px 15px rgba(220, 53, 69, 0.3)';
        }
    }
}

// Enhanced uptime formatting
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m ${secs}s`;
    } else if (minutes > 0) {
        return `${minutes}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

// Enhanced refresh function with loading states
window.refreshData = function() {
    console.log('🔄 Manual refresh initiated');
    
    // Add loading states to elements
    const elements = ['current-temp', 'current-humidity', 'current-gas', 'current-pid'];
    elements.forEach(id => {
        const element = document.getElementById(id);
        if (element) {
            element.classList.add('loading');
        }
    });
    
    // Refresh after a short delay to show loading animation
    setTimeout(() => {
        location.reload();
    }, 500);
};

// Export functions for global access
window.poultrixEdgeAI = {
    refreshData: window.refreshData,
    updateAIInsights: processAIInsights,
    version: '2.0.0-competition'
};

console.log('🏆 Poultrix Edge AI Dashboard loaded successfully for Edge Impulse Competition 2025');