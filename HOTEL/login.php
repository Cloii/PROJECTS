
	<?php
// Start session
session_start();

// Dummy username and password (you should use a database in a real scenario)
$valid_username = "bil";
$valid_password = "123";

// Check if form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Get username and password from the form
    $username = $_POST["username"];
    $password = $_POST["password"];

    // Check if username and password are valid
    if ($username === $valid_username && $password === $valid_password) {
        // Set session variables
        $_SESSION["username"] = $username;
        // Redirect to another page after successful login
        header("Location: Admin.php");
        exit();
    } else {
        // Redirect back to login page with error message
        header("Location: login.html?error=invalid_credentials");
        exit();
    }
} else {
    // If someone tries to directly access this page, redirect them to the login page
    header("Location: login.html");
    exit();
}
?>
