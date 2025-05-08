<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Admin Booking and Reservation</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f0f0f0;
        }

        .navbar {
            background-color: #333;
            overflow: hidden;
            color: white;
            padding: 14px 20px;
            text-align: center;
        }

        .navbar a {
            color: white;
            text-align: center;
            padding: 14px 20px;
            text-decoration: none;
            display: inline-block;
        }

        .navbar a:hover {
            background-color: #ddd;
            color: black;
        }

        .sidebar {
            height: 100%;
            width: 200px;
            position: fixed;
            top: 0;
            left: 0;
            background-color: #333;
            padding-top: 20px;
        }

        .sidebar a {
            padding: 15px 20px;
            text-decoration: none;
            font-size: 18px;
            color: white;
            display: block;
        }

        .sidebar a:hover {
            background-color: #575757;
        }

        .main-content {
            margin-left: 220px;
            padding: 20px;
            background-color: #f0f0f0;
            height: 100vh;
        }

        h1, h2 {
            text-align: center;
            color: #333;
        }

        .form-container, .table-container {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-top: 10px;
            color: #333;
        }

        input[type="text"], input[type="number"], input[type="submit"] {
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
        }
        
        input[type="submit"]:hover {
            background: #555;
        }

        table {
            width: 100%;
            border-collapse: collapse;
        }

        th, td {
            border: 1px solid #ddd;
            padding: 8px;
            text-align: left;
        }

        th {
            background-color: #f2f2f2;
        }

        tr:nth-child(even) {
            background-color: #f9f9f9;
        }

        .search-form {
            margin: 20px 0;
            text-align: center;
        }

        .search-form input[type="text"] {
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            width: 200px;
        }

        .search-form button {
            padding: 8px 16px;
            background-color: #333;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            margin-left: 10px;
        }

        .search-form button:hover {
            background-color: #555;
        }

        .action-links a {
            color: blue;
            text-decoration: none;
        }

        .action-links a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>

<div class="navbar">
    ADMIN DASHBOARD
</div>

<div class="sidebar">
    <a href="Admin.php">Admin</a>
    <a href="logout.php">Logout</a>
</div>

<div class="main-content">

    <div class="table-container">
        <h2>Data Entries</h2>
        <form class="search-form" method="GET" action="">
            <input type="text" name="q" placeholder="Search by name">
            <button type="submit">Search</button>
        </form>

        <table>
            <tr>
                <th>Room Type</th>
                <th>Room no.</th>
                <th>Name</th>
                <th>Address</th>
                <th>Contact</th>
                <th>Email</th>
                <th>Gender</th>
                <th>Check-in Date</th>
                <th>Check-out Date</th>
                <th colspan="2">Action</th>
            </tr>

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

            // Handle search query
            $search = isset($_GET['q']) ? $_GET['q'] : '';

            // Fetch data from the database
            if ($search) {
                $sql = "SELECT roomtype, roomnumb, name, address, contact, email, gender, check_in_date, check_out_date FROM guests WHERE name LIKE ?";
                $stmt = $conn->prepare($sql);
                $search_param = "%" . $search . "%";
                $stmt->bind_param("s", $search_param);
            } else {
                $sql = "SELECT roomtype, roomnumb, name, address, contact, email, gender, check_in_date, check_out_date FROM guests";
                $stmt = $conn->prepare($sql);
            }

            $stmt->execute();
            $result = $stmt->get_result();

            if ($result && $result->num_rows > 0) {
                while ($row = $result->fetch_assoc()) {
                    echo "<tr>";
                    echo "<td>" . $row['roomtype'] . "</td>";
                    echo "<td>" . $row['roomnumb'] . "</td>";
                    echo "<td>" . $row['name'] . "</td>";
                    echo "<td>" . $row['address'] . "</td>";
                    echo "<td>" . $row['contact'] . "</td>";
                    echo "<td>" . $row['email'] . "</td>";
                    echo "<td>" . $row['gender'] . "</td>";
                    echo "<td>" . $row['check_in_date'] . "</td>";
                    echo "<td>" . $row['check_out_date'] . "</td>";
                    echo "<td class='action-links'><a href='update_delete.php?edit=" . $row['roomtype'] . "'>UPDATE</a></td>";
                    echo "<td class='action-links'><a href='delete.php?delete=" . $row['roomtype'] . "'>DELETE</a></td>";
                    echo "</tr>";
                }
            } else {
                echo "<tr><td colspan='11'>No data found</td></tr>";
            }

            $stmt->close();
            $conn->close();
            ?>
        </table>
    </div>
</div>

</body>
</html>
