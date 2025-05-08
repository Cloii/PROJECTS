<?php
// Database connection details
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "hotel";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Check if the form is submitted
if (isset($_POST['submit'])) {
    // Get the form data
    $roomtype = $_POST['room_type'];
    $roomnumb = $_POST['room_number'];
    $name = $_POST['name'];
    $address = $_POST['address'];
    $contact = $_POST['contact'];
    $email = $_POST['email'];
    $gender = $_POST['gender'];
    $check_in_date = $_POST['check_in_date']; // Retrieve check-in date from the form
    $check_out_date = $_POST['check_out_date']; // Retrieve check-out date from the form

    // Check if the room is already booked for the selected dates
    $check_query = "SELECT * FROM guests WHERE roomnumb = ? AND check_in_date = ? AND check_out_date = ?";
    $check_stmt = $conn->prepare($check_query);
    $check_stmt->bind_param("iss", $roomnumb, $check_in_date, $check_out_date);
    $check_stmt->execute();
    $check_result = $check_stmt->get_result();

    if ($check_result->num_rows > 0) {
        echo '<script>alert("The room is already booked for the selected dates. Please choose another room or different dates."); window.location.href = "booking_form.html";</script>';
        exit(); // Exit if the room is already booked for the selected dates
    }

    // Prepare and bind the insert statement
    $insert_stmt = $conn->prepare("INSERT INTO guests (roomtype, roomnumb, name, address, contact, email, gender, check_in_date, check_out_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    $insert_stmt->bind_param("sisssssss", $roomtype, $roomnumb, $name, $address, $contact, $email, $gender, $check_in_date, $check_out_date);

    // Execute the insert statement
    if ($insert_stmt->execute()) {
        echo '<script>alert("Booking successful!"); window.location.href = "Home.html";</script>';
        exit(); // Exit after redirection
    } else {
        echo "Error: " . $insert_stmt->error;
    }

    // Close statement
    $insert_stmt->close();
}

// Close database connection
$conn->close();
?>
