CREATE DATABASE IF NOT EXISTS water_drone_db;
USE water_drone_db;

CREATE TABLE IF NOT EXISTS drone_logs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    kualitas_air DECIMAL(10, 2), -- Water Quality (e.g., pH, Turbidity)
    tahan DECIMAL(10, 2),        -- Resistance/Soil/Other metrics (Tahan)
    udara DECIMAL(10, 2),        -- Air Quality/Temperature
    daya_listrik DECIMAL(10, 2)  -- Power/Battery Level
);


-- Jalankan di Query phpMyAdmin

-- 1. Buat Database water_drone_db
-- 2. Copy semua code yang ada di file ini lalu jalankan