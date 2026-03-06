<?php
header("Content-Type: application/json");
header("Access-Control-Allow-Origin: *");
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
mysqli_report(MYSQLI_REPORT_OFF);

// === KONFIGURASI DATABASE ===
$servername = "localhost";
$username   = "underwat_daffa_water";
$password   = "DDKg4Br3WwYBeTkyg7Dm";
$dbname     = "underwat_daffa_water";

// === KONEKSI DATABASE ===
$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed: " . $conn->connect_error]));
}

// ============================================================
// ENDPOINT 1: INSERT DATA DARI DRONE
// Sensor: pH (kualitas_air) + Turbidity (tahan) + Baterai (daya_listrik) + Suhu (suhu)
// Contoh: api.php?kualitas_air=7.20&tahan=312.50&daya_listrik=100&suhu=25.5
// ============================================================
    if (isset($_GET['kualitas_air']) && isset($_GET['tahan']) && isset($_GET['daya_listrik']) && isset($_GET['suhu'])) {
    
        $kualitas_air = floatval($_GET['kualitas_air']);
        $tahan        = floatval($_GET['tahan']);
        $daya_listrik = floatval($_GET['daya_listrik']);
        $suhu         = floatval($_GET['suhu']);
    
        $stmt = $conn->prepare("INSERT INTO drone_logs (kualitas_air, tahan, daya_listrik, suhu) VALUES (?, ?, ?, ?)");
        $stmt->bind_param("dddd", $kualitas_air, $tahan, $daya_listrik, $suhu);

    if ($stmt->execute()) {
        echo json_encode([
            "status"  => "success",
            "message" => "Data inserted successfully",
            "id"      => $conn->insert_id,
            "data"    => [
                "pH"          => $kualitas_air,
                "turbidity"   => $tahan,
                "battery"     => $daya_listrik,
                "temperature" => $suhu
            ]
        ]);
    } else {
        echo json_encode(["status" => "error", "message" => "Insert error: " . $stmt->error]);
    }

    $stmt->close();

// ============================================================
// ENDPOINT 2: AMBIL DATA TERBARU (live monitoring)
// Contoh: api.php?get_latest=true
// ============================================================
} elseif (isset($_GET['get_latest'])) {

    $sql    = "SELECT * FROM drone_logs ORDER BY id DESC LIMIT 1";
    $result = $conn->query($sql);

    if ($result && $result->num_rows > 0) {
        echo json_encode($result->fetch_assoc());
    } else {
        echo json_encode(["status" => "empty", "message" => "No data found"]);
    }

// ============================================================
// ENDPOINT 3: KONTROL MOTOR (SET COMMAND)
// Contoh: api.php?set_command=START|STOP
// ============================================================
} elseif (isset($_GET['set_command'])) {

    $cmd = strtoupper($_GET['set_command']);
    if (!in_array($cmd, ['START', 'STOP'])) {
        die(json_encode(["status" => "error", "message" => "Command invalid"]));
    }

    $stmt = $conn->prepare("UPDATE drone_commands SET command = ? WHERE id = 1");
    $stmt->bind_param("s", $cmd);

    if ($stmt->execute()) {
        echo json_encode(["status" => "success", "command" => $cmd]);
    } else {
        echo json_encode(["status" => "error", "message" => $stmt->error]);
    }
    $stmt->close();

// ============================================================
// ENDPOINT 4: POLLING COMMAND UNTUK DRONE
// Contoh: api.php?get_command=true
// ============================================================
} elseif (isset($_GET['get_command'])) {

    $result = $conn->query("SELECT command FROM drone_commands WHERE id = 1");
    if ($result && $result->num_rows > 0) {
        $row = $result->fetch_assoc();
        echo json_encode(["status" => "success", "command" => $row['command']]);
    } else {
        echo json_encode(["status" => "error", "message" => "No command found"]);
    }

// ============================================================
// DEFAULT: Tampilkan daftar endpoint yang tersedia
// ============================================================
} else {
    echo json_encode([
        "status"    => "error",
        "message"   => "Parameter tidak dikenali.",
        "endpoints" => [
            "INSERT data drone" => "api.php?kualitas_air=7.2&tahan=312&daya_listrik=100&suhu=25.5",
            "GET latest data"   => "api.php?get_latest=true",
            "SET command"       => "api.php?set_command=START|STOP",
            "GET command"       => "api.php?get_command=true"
        ]
    ]);
}

$conn->close();
?>