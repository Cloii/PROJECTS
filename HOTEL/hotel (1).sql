-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: May 20, 2024 at 05:41 PM
-- Server version: 10.4.32-MariaDB
-- PHP Version: 8.2.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `hotel`
--

-- --------------------------------------------------------

--
-- Table structure for table `guests`
--

CREATE TABLE `guests` (
  `id` int(11) NOT NULL,
  `roomtype` varchar(11) NOT NULL,
  `roomnumb` int(11) NOT NULL,
  `name` varchar(30) NOT NULL,
  `address` varchar(30) NOT NULL,
  `contact` varchar(30) NOT NULL,
  `email` varchar(30) NOT NULL,
  `gender` varchar(30) NOT NULL,
  `check_in_date` date NOT NULL,
  `check_out_date` date NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `guests`
--

INSERT INTO `guests` (`id`, `roomtype`, `roomnumb`, `name`, `address`, `contact`, `email`, `gender`, `check_in_date`, `check_out_date`) VALUES
(53, 'Deluxe', 2, 'Jamesssz', '123 Main St, Citydddd', '1234567890ddd', 'james.smith@example.com', 'male', '2024-05-21', '2024-05-22'),
(55, 'Suite', 10, 'BIL Clnth222', '369 Walnut St, Mountains', '7779991111sdsds', 'ethan.garcia@example.com', 'female', '2024-05-21', '2024-05-22'),
(56, 'Deluxe', 1, 'James Smith', '123 Main St, Citydddd', '1234567890', 'james.smith@example.com', 'female', '2024-05-21', '2024-05-22'),
(57, 'Suite', 8, 'Ethan Garcia', '369 Walnut St, Mountains', '7779991111sdsds', 'ethan.garcia@example.com', 'male', '2024-05-21', '2024-05-22'),
(58, 'Suite', 3, 'bil clinth', 'dauis', '0932923', 'bilclinth04@gmail.com', 'male', '2024-05-21', '2024-05-22');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `guests`
--
ALTER TABLE `guests`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `guests`
--
ALTER TABLE `guests`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=59;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
