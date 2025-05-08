<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hotel Booking and Reservation</title>
	<<script>
        // Function to show confirmation message and redirect
        function showConfirmation() {
            alert("Booking successful!");
            window.location.href = "Home.html";
        }
    </script>   
    <style>
        body {
            font-family: 'Arial', sans-serif;
            margin: 0;
            padding: 0;
            background-image: url('https://imageio.forbes.com/specials-images/imageserve/65280003a36cd6aea36f399a/Maldives-hotel-beach-resort-on-tropical-Island-with-aerial-drone-view/0x0.jpg?format=jpg&height=1406&width=2501');
            background-size: cover;
            background-position: center;
            background-attachment: fixed;
        }

        .navbar {
            background: rgba(0, 0, 0, 0.7);
            overflow: hidden;
            margin-bottom: 20px;
            text-align: center;
            padding: 10px 0;
        }

        .navbar a {
            color: white;
            text-align: center;
            padding: 14px 20px;
            text-decoration: none;
            display: inline-block;
        }

        .navbar a:hover {
            background-color: rgba(255, 255, 255, 0.7);
            color: black;
        }

        .content {
            width: 100%;
            max-width: 1200px;
            margin: auto;
            padding: 20px;
            background: rgba(255, 255, 255, 0.8);
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);
        }

        .form-container, .table-container {
            margin-bottom: 40px;
        }

        h1, h2 {
            text-align: center;
            color: black;
        }

        .form-container form, .search-form form {
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .form-container label, .search-form input, .form-container input[type="text"], .form-container input[type="number"], .form-container input[type="email"], .form-container select, .form-container input[type="submit"] {
            width: 100%;
            max-width: 500px;
            padding: 10px;
            margin: 5px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
        }

        .form-container input[type="submit"] {
            background: #333;
            color: white;
            cursor: pointer;
            font-weight: bold;
        }

        .form-container input[type="submit"]:hover {
            background: #555;
        }
    </style>
</head>
<body>

<div class="navbar">
    <a href="Home.html">Home</a>
    <a href="room_availability.php">Reservation</a>
    <a href="login.php">Admin Login</a>
</div>

<div class="content">
    <div class="form-container">
        <h1>BOOKING FORM</h1>
        <form action="insert1.php" method="POST" enctype="multipart/form-data" onsubmit="showConfirmation(event)">
            <?php
            // Retrieve parameters from the URL
            $room_number = $_GET['room_number'];
            $check_in_date = $_GET['check_in_date'];
            $check_out_date = $_GET['check_out_date'];
            $room_type = $_GET['room_type'];
            ?>
            <input type="hidden" name="room_number" value="<?php echo $room_number; ?>">
            <input type="hidden" name="check_in_date" value="<?php echo $check_in_date; ?>">
            <input type="hidden" name="check_out_date" value="<?php echo $check_out_date; ?>">
            Room Type: <input type="text" name="room_type" value="<?php echo $room_type; ?>" readonly><br>
            Room Number: <input type="text" name="room_number" value="<?php echo $room_number; ?>" readonly><br>
            Check-in Date: <input type="text" name="check_in_date" value="<?php echo $check_in_date; ?>" readonly><br>
            Check-out Date: <input type="text" name="check_out_date" value="<?php echo $check_out_date; ?>" readonly><br>

            <label for="name">Full Name</label>
            <input type="text" id="name" name="name" required>

            <label for="address">Address</label>
            <input type="text" id="address" name="address" required>

            <label for="contact">Contact Number</label>
            <input type="text" id="contact" name="contact" required>

            <label for="email">Email</label>
            <input type="email" id="email" name="email" required>

            <label for="gender">Gender</label>
            <select id="gender" name="gender" required>
                <option value="male">Male</option>
                <option value="female">Female</option>
                <option value="other">Other</option>
            </select>

            <input type="submit" name="submit" value="BOOK">
        </form>
    </div>
</div>

</body>
</html>
