<?php
// Database connection
$conn = mysqli_connect('localhost', 'root', '', 'hotel');
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

if (isset($_GET['delete'])) {
    $roomtype = $_GET['delete'];
    $sql = "DELETE FROM guests WHERE roomtype = ? LIMIT 1"; // Limiting deletion to only one row
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("s", $roomtype);
    $stmt->execute();
    $stmt->close();
    header("Location: admin.php"); // Redirect to admin.php after deletion
    exit();
}

mysqli_close($conn);
?>
