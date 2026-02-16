<?php
header("Content-Type: application/json");
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
mysqli_report(MYSQLI_REPORT_OFF); // Matikan exception agar error bisa ditangkap manual


// Ubah sesuai data di database Hosting
$servername = "localhost"; 
$username = "zakkalmy_water_drone"; 
$password = "xvyhzmkZsr5fruZTnQsV"; 
$dbname = "zakkalmy_water_drone"; 

// Connect to database
$conn = new mysqli($servername, $username, $password, $dbname);
    
// Check connection
if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed: " . $conn->connect_error]));
}

// Get parameters from URL (GET request)
$kualitas_air = isset($_GET['kualitas_air']) ? floatval($_GET['kualitas_air']) : null;
$tahan = isset($_GET['tahan']) ? floatval($_GET['tahan']) : null;
$udara = isset($_GET['udara']) ? floatval($_GET['udara']) : null;
$daya_listrik = isset($_GET['daya_listrik']) ? floatval($_GET['daya_listrik']) : null;

// Validate that required parameters are present (allow 0 values, check for null)
if ($kualitas_air !== null && $tahan !== null && $udara !== null && $daya_listrik !== null) {
    
    // Prepare and bind
    $stmt = $conn->prepare("INSERT INTO drone_logs (kualitas_air, tahan, udara, daya_listrik) VALUES (?, ?, ?, ?)");
    $stmt->bind_param("dddd", $kualitas_air, $tahan, $udara, $daya_listrik);

    if ($stmt->execute()) {
        echo json_encode(["status" => "success", "message" => "Data inserted successfully"]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error: " . $stmt->error]);
    }

    $stmt->close();
} elseif (isset($_GET['get_latest'])) {
    // Fetch latest data
    $sql = "SELECT * FROM drone_logs ORDER BY id DESC LIMIT 1";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        echo json_encode($result->fetch_assoc());
    } else {
        echo json_encode(["status" => "empty", "message" => "No data found"]);
    }
} else {
    echo json_encode([
        "status" => "error", 
        "message" => "Missing parameters. Required: kualitas_air, tahan, udara, daya_listrik OR get_latest=true"
    ]);
}

$conn->close();
?>