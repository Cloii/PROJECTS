<?php
// Function to check room availability
function checkRoomAvailability($check_in_date, $check_out_date) {
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

    // Fetch booked rooms for the selected dates
    $sql = "SELECT roomnumb FROM guests WHERE check_out_date > ? AND check_in_date < ?";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("ss", $check_in_date, $check_out_date);
    $stmt->execute();
    $result = $stmt->get_result();

    $booked_rooms = array();
    while ($row = $result->fetch_assoc()) {
        $booked_rooms[] = $row['roomnumb'];
    }

    // Close statement
    $stmt->close();

    // Close database connection
    $conn->close();

    // Initialize array to store available rooms
    $available_rooms = array();

    // Generate available rooms
    for ($i = 1; $i <= 10; $i++) {
        if (!in_array((string)$i, $booked_rooms)) {
            $available_rooms[] = array(
                "room_number" => $i,
                "room_type" => ($i % 2 == 0) ? "Deluxe" : "Suite"
            );
        }
    }

    return $available_rooms;
}

// Check if form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST" && isset($_POST["check_availability"])) {
    // Retrieve check-in and check-out dates from form
    $check_in_date = $_POST["check_in_date"];
    $check_out_date = $_POST["check_out_date"];

    // Check if selected dates are on weekends
    $is_weekend = (date('N', strtotime($check_in_date)) >= 6) || (date('N', strtotime($check_out_date)) >= 6);

    // Check room availability only if not on weekends
    if (!$is_weekend) {
        $available_rooms = checkRoomAvailability($check_in_date, $check_out_date);
    }
}

// Insert booking into the database
if ($_SERVER["REQUEST_METHOD"] == "GET" && isset($_GET["submit"])) {
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

    // Retrieve form data
    $room_number = $_GET['room_number'];
    $check_in_date = $_GET['check_in_date'];
    $check_out_date = $_GET['check_out_date'];
    $room_type = $_GET['room_type'];
    $name = $_GET['name'];
    $address = $_GET['address'];
    $contact = $_GET['contact'];
    $email = $_GET['email'];
    $gender = $_GET['gender'];

    // Prepare and bind the insert statement
    $stmt = $conn->prepare("INSERT INTO guests (roomnumb, roomtype, check_in_date, check_out_date, name, address, contact, email, gender) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    $stmt->bind_param("issssssss", $room_number, $room_type, $check_in_date, $check_out_date, $name, $address, $contact, $email, $gender);

    // Execute the statement
    if ($stmt->execute()) {
        echo "<script>alert('Booking successful!'); window.location.href = 'Home.html';</script>";
        exit();
    } else {
        echo "Error: " . $stmt->error;
    }

    // Close statement
    $stmt->close();

    // Close database connection
    $conn->close();
}
?>


<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Room Availability</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f3f3f3;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        h2 {
            text-align: center;
            margin-bottom: 20px;
        }
        form {
            text-align: center;
            margin-bottom: 20px;
        }
        label {
            font-weight: bold;
        }
        input[type="date"] {
            padding: 8px;
            margin: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        input[type="submit"] {
            padding: 10px 20px;
            background-color: #007bff;
            color: #fff;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        input[type="submit"]:hover {
            background-color: #0056b3;
        }
        .rooms {
            display: flex;
            justify-content: space-around;
            margin-top: 20px;
        }
        .room {
            border: 1px solid #ccc;
            padding: 20px;
            text-align: center;
            width: 45%;
            background-color: #fff;
            border-radius: 10px;
        }
        .room img {
            max-width: 100%;
            height: auto;
            border-radius: 5px;
            margin-bottom: 10px;
        }
        .room h3 {
            color: #007bff;
        }
        .room p {
            color: #333;
            margin-bottom: 15px;
        }
        .room ul {
            list-style-type: none;
            padding: 0;
            margin: 0;
            text-align: left;
        }
        .room ul li {
            margin-bottom: 5px;
        }
        .room a {
            color: #007bff;
            text-decoration: none;
        }
        .room a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Check Room Availability</h2>
        <form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>">
            <label for="check_in_date">Check-in Date:</label>
            <input type="date" id="check_in_date" name="check_in_date" value="<?php echo isset($check_in_date) ? $check_in_date : ''; ?>" required>
            <br>
            <label for="check_out_date">Check-out Date:</label>
            <input type="date" id="check_out_date" name="check_out_date" value="<?php echo isset($check_out_date) ? $check_out_date : ''; ?>" required>
            <br>
            <input type="submit" name="check_availability" value="Check Availability">
        </form>

        <?php if ($_SERVER["REQUEST_METHOD"] == "POST") : ?>
            <?php if ($is_weekend) : ?>
                <h3>Warning: No available rooms on this date.</h3>
            <?php elseif (isset($available_rooms)) : ?>
                <?php if (count($available_rooms) > 0) : ?>
                    <h3>Available Rooms:</h3>
                    <div class="rooms">
                        <?php
                        $deluxe_rooms = array_filter($available_rooms, function($room) {
                            return $room['room_type'] == 'Deluxe';
                        });
                        $suite_rooms = array_filter($available_rooms, function($room) {
                            return $room['room_type'] == 'Suite';
                        });
                        ?>
                        <div class="room">
                            <img src="https://courtmeridian.com/wp-content/uploads/2023/04/12-A.jpg-Super-Deluxe-with-Veranda-with-2-Queen-Size-Beds-1.jpg" alt="Deluxe Room Image">
                            <h3>Deluxe Room</h3>
                            <p>Experience luxury and comfort in our deluxe rooms with stunning views and modern amenities. Our Deluxe rooms feature:</p>
                            <ul>
                                <li>Queen-size beds with premium bedding</li>
                                <li>Spacious ensuite bathroom with bathtub</li>
                                <li>High-speed Wi-Fi</li>
                                <li>Flat-screen TV with cable channels</li>
                                <li>Mini bar</li>
                            </ul>
                            <?php if (count($deluxe_rooms) > 0) : ?>
                                <h4>Available Deluxe Rooms:</h4>
                                <ul>
                                    <?php foreach ($deluxe_rooms as $room) : ?>
                                        <li>Room <?php echo $room['room_number']; ?> - <a href='form1.php?room_number=<?php echo $room['room_number']; ?>&check_in_date=<?php echo $check_in_date; ?>&check_out_date=<?php echo $check_out_date; ?>&room_type=Deluxe'>Book Now</a></li>
                                    <?php endforeach; ?>
                                </ul>
                            <?php else : ?>
                                <p>All Deluxe Rooms are booked.</p>
                            <?php endif; ?>
                        </div>
                        <div class="room">
                            <img src="https://www.venetianlasvegas.com/content/dam/venetian/suites/venetian/prima-grand-one-bedroom/bar-1_2000x812.jpg.resize.0.0.1200.630.jpg?ignorecache=true" alt="Suite Image">
                            <h3>Suite</h3>
                            <p>Our suites offer spacious living areas and premium facilities for an unforgettable stay. Our Suites feature:</p>
                            <ul>
                                <li>King-size beds with luxurious linens</li>
                                <li>Separate living and dining areas</li>
                                <li>Private balcony with panoramic views</li>
                                <li>Spa-style bathroom with Jacuzzi tub</li>
                                <li>In-room coffee machine</li>
                            </ul>
                            <?php if (count($suite_rooms) > 0) : ?>
                                <h4>Available Suites:</h4>
                                <ul>
                                    <?php foreach ($suite_rooms as $room) : ?>
                                        <li>Room <?php echo $room['room_number']; ?> - <a href='form1.php?room_number=<?php echo $room['room_number']; ?>&check_in_date=<?php echo $check_in_date; ?>&check_out_date=<?php echo $check_out_date; ?>&room_type=Suite'>Book Now</a></li>
                                    <?php endforeach; ?>
                                </ul>
                            <?php else : ?>
                                <p>All Suites are booked.</p>
                            <?php endif; ?>
                        </div>
                    </div>
                <?php else : ?>
                    <h3>No Available Rooms for the selected dates.</h3>
                <?php endif; ?>
            <?php endif; ?>
        <?php endif; ?>
    </div>
</body>
</html>

