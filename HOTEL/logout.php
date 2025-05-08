<?php
// Start session
session_start();

// Destroy session
session_destroy();

// Redirect to form page
header("Location: Home.html");
exit;
?>
