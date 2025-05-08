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

// Handle update operation
if (isset($_POST['update'])) {
    $roomtype = $_POST['roomtype'];
    $roomnumb = $_POST['roomnumb'];
    $name = $_POST['name'];
    $address = $_POST['address'];
    $contact = $_POST['contact'];
    $email = $_POST['email'];
    $gender = $_POST['gender'];

    $sql = "UPDATE guests SET name = ?, address = ?, contact = ?, email = ?, gender = ? WHERE roomtype = ? AND roomnumb = ? LIMIT 1";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("ssssssi", $name, $address, $contact, $email, $gender, $roomtype, $roomnumb);

    if ($stmt->execute()) {
        echo "Data updated successfully.";
        header("Location: admin.php"); // Redirect to admin.php after update
        exit();
    } else {
        echo "Error: " . $stmt->error;
    }

    $stmt->close();
}

// Fetch the existing data for update
if (isset($_GET['edit'])) {
    $roomtype = $_GET['edit'];
    $sql = "SELECT * FROM guests WHERE roomtype = ? LIMIT 1";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("s", $roomtype);
    $stmt->execute();
    $result = $stmt->get_result();
    $row = $result->fetch_assoc();

    if ($row) {
        $roomnumb = $row['roomnumb'];
        $name = $row['name'];
        $address = $row['address'];
        $contact = $row['contact'];
        $email = $row['email'];
        $gender = $row['gender'];
    }

    $stmt->close();
} else {
    $roomtype = '';
    $roomnumb = '';
    $name = '';
    $address = '';
    $contact = '';
    $email = '';
    $gender = '';
}

$conn->close();
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Update/Delete Guest</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 0;
        }

        .form-container {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);
            margin: 50px auto;
            width: 50%;
        }

        label {
            display: block;
            margin-top: 10px;
            color: #333;
        }

        input[type="text"], input[type="number"], input[type="email"], select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            margin-bottom: 10px;
        }

        input[type="submit"] {
            background: #333;
            color: white;
            cursor: pointer;
            font-weight: bold;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
        }

        input[type="submit"]:hover {
            background: #555;
        }
    </style>
</head> 
<body>

<div class="form-container">
    <h1>Update Guest</h1>
    <form action="update_delete.php" method="POST">
        <label for="roomtype">Room Type</label>
        <input type="text" id="roomtype" name="roomtype" value="<?php echo $roomtype; ?>" readonly>

        <label for="roomnumb">Room Number</label>
        <input type="number" id="roomnumb" name="roomnumb" value="<?php echo $roomnumb; ?>" required>

        <label for="name">Full Name</label>
        <input type="text" id="name" name="name" value="<?php echo $name; ?>" required>

        <label for="address">Address</label>
        <input type="text" id="address" name="address" value="<?php echo $address; ?>" required>

        <label for="contact">Contact Number</label>
        <input type="text" id="contact" name="contact" value="<?php echo $contact; ?>" required>

        <label for="email">Email</label>
        <input type="email" id="email" name="email" value="<?php echo $email; ?>" required>

        <label for="gender">Gender</label>
        <select id="gender" name="gender" required>
            <option value="male" <?php if ($gender == 'male') echo 'selected'; ?>>Male</option>
            <option value="female" <?php if ($gender == 'female') echo 'selected'; ?>>Female</option>
            <option value="other" <?php if ($gender == 'other') echo 'selected'; ?>>Other</option>
        </select>

        <input type="submit" name="update" value="Update">
    </form>
</div>

</body>
</html>
