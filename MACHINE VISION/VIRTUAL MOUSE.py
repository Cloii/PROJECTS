import cv2
import mediapipe as mp
import pyautogui
import time
import numpy as np
from screeninfo import get_monitors
import platform
import pygetwindow as gw
import subprocess
import threading
import math

# Initialize video capture
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
cap.set(cv2.CAP_PROP_FPS, 30)
if not cap.isOpened():
    print("Error: Could not open video capture.")
    exit()

# Initialize MediaPipe Hands
mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
hands = mp_hands.Hands(
    max_num_hands=2,
    min_detection_confidence=0.9,
    min_tracking_confidence=0.7,
    static_image_mode=False
)

# PyAutoGUI settings
pyautogui.FAILSAFE = True
pyautogui.PAUSE = 0.001

# Monitor setup
try:
    monitors = get_monitors()
    if not monitors:
        print("Error: No monitors detected.")
        exit()
    virtual_width = max(monitor.x + monitor.width for monitor in monitors)
    virtual_height = max(monitor.y + monitor.height for monitor in monitors)
    virtual_min_x = min(monitor.x for monitor in monitors)
    virtual_min_y = min(monitor.y for monitor in monitors)
    print(f"Virtual screen: width={virtual_width}, height={virtual_height}, min_x={virtual_min_x}, min_y={virtual_min_y}")
    print(f"Monitors detected: {[(m.x, m.y, m.width, m.height) for m in monitors]}")
except Exception as e:
    print(f"Error detecting monitors: {e}")
    exit()

# Initialize variables
pTime = 0
prev_x, prev_y = 0, 0
click_distance_threshold = 15
middle_click_distance_threshold = 15
last_click_time = 0
last_middle_click_time = 0
click_debounce_time = 0.1
is_dragging = False
drag_start_time = 0
drag_hold_time = 0.2
swipe_history = []
swipe_time_window = 0.3
swipe_velocity_threshold = 300  # Tuned for horizontal swipes
swipe_distance_threshold = 45   # Tuned for sensitivity
swipe_min_duration = 0.1       # Minimum duration for a swipe
swipe_confirmation_points = 8  # Number of points for swipe confirmation
swipe_vertical_threshold = 20  # Max vertical movement for horizontal swipe
last_swipe_time = 0
swipe_debounce_time = 0.25
last_window_move_time = 0
window_move_debounce_time = 0.3
frame_skip_counter = 0
min_fps_threshold = 15
prev_velocity = 0
velocity_smoothing_factor = 0.4
monitor_cache = {}  # Cache for monitor assignments

# Platform detection
os_type = platform.system()

def move_window_to_monitor(target_monitor):
    """Move the active window to the center of the target monitor asynchronously."""
    def async_move():
        global last_window_move_time
        current_time = time.time()
        if current_time - last_window_move_time < window_move_debounce_time:
            return
        try:
            print(f"Moving window to monitor at x={target_monitor.x}, y={target_monitor.y}")
            if os_type == "Windows":
                win = gw.getActiveWindow()
                if not win or win.isMinimized or not win.title or "Taskbar" in win.title:
                    print("Invalid window for movement.")
                    return
                win.restore()
                target_x = target_monitor.x + (target_monitor.width - min(1200, target_monitor.width - 100)) // 2
                target_y = target_monitor.y + (target_monitor.height - min(800, target_monitor.height - 100)) // 2
                win.moveTo(target_x, target_y)
                win.resizeTo(min(1200, target_monitor.width - 100), min(800, target_monitor.height - 100))
                win.maximize()
            elif os_type == "Darwin":
                from AppKit import NSAppleScript
                script = f'''
                tell application "System Events"
                    if not (exists front window of frontmost application) then
                        return
                    end if
                    tell frontmost application
                        try
                            set position of front window to {{{target_monitor.x + (target_monitor.width - min(1200, {target_monitor.width - 100})) / 2}, {target_monitor.y + (target_monitor.height - min(800, {target_monitor.height - 100})) / 2}}}
                            set size of front window to {{min(1200, {target_monitor.width - 100}), min(800, {target_monitor.height - 100})}}
                        end try
                    end tell
                end tell
                '''
                NSAppleScript.alloc().initWithSource_(script).executeAndReturnError_(None)
            elif os_type == "Linux":
                active_window = subprocess.run(["xdotool", "getactivewindow"], capture_output=True, text=True)
                if active_window.stdout.strip():
                    target_x = target_monitor.x + (target_monitor.width - min(1200, target_monitor.width - 100)) // 2
                    target_y = target_monitor.y + (target_monitor.height - min(800, target_monitor.height - 100)) // 2
                    subprocess.run(["xdotool", "getactivewindow", "windowmove", str(target_x), str(target_y)])
                    subprocess.run(["xdotool", "getactivewindow", "windowsize", str(min(1200, target_monitor.width - 100)), str(min(800, target_monitor.height - 100))])
            last_window_move_time = current_time
            print(f"Window moved to monitor at x={target_monitor.x}, y={target_monitor.y}")
        except Exception as e:
            print(f"Error moving window: {e}")
    threading.Thread(target=async_move, daemon=True).start()

def get_target_monitor(current_x, current_y, direction):
    """Determine target monitor for horizontal swipes with improved logic."""
    cache_key = (current_x, current_y, direction)
    if cache_key in monitor_cache:
        return monitor_cache[cache_key]
    
    current_monitor = None
    for monitor in monitors:
        if (monitor.x <= current_x < monitor.x + monitor.width and
                monitor.y <= current_y < monitor.y + monitor.height):
            current_monitor = monitor
            break
    
    if not current_monitor:
        current_monitor = min(monitors, key=lambda m: ((m.x + m.width/2 - current_x)**2 + (m.y + m.height/2 - current_y)**2)**0.5)
        print(f"Fallback: Selected closest monitor at x={current_monitor.x}, y={current_monitor.y}")

    candidates = []
    if direction == "left":
        candidates = [m for m in monitors if m.x + m.width <= current_monitor.x + 10]
    elif direction == "right":
        candidates = [m for m in monitors if m.x >= current_monitor.x + current_monitor.width - 10]
    
    if not candidates:
        print(f"No valid target monitor found for {direction} swipe.")
        return None
    
    target = min(candidates, key=lambda m: (
        abs(m.y + m.height/2 - current_monitor.y - current_monitor.height/2),
        abs(m.x - (current_monitor.x + (current_monitor.width if direction == "right" else -m.width)))
    ))
    
    monitor_cache[cache_key] = target
    return target

def get_hand_centroid(hand_lm, w, h):
    """Calculate hand centroid with refined weights."""
    x_sum, y_sum, weight_sum = 0, 0, 0
    for i, lm in enumerate(hand_lm.landmark):
        weight = 2.0 if i in [4, 8, 12, 16, 20] else 0.8
        x_sum += lm.x * w * weight
        y_sum += lm.y * h * weight
        weight_sum += weight
    return x_sum / weight_sum, y_sum / weight_sum

def estimate_hand_distance(hand_lm):
    """Estimate hand distance using thumb-index distance."""
    thumb_tip = hand_lm.landmark[4]
    index_tip = hand_lm.landmark[8]
    return ((thumb_tip.x - index_tip.x)**2 + (thumb_tip.y - index_tip.y)**2)**0.5

def get_hand_orientation(hand_lm):
    """Calculate hand orientation for horizontal swipes."""
    wrist = hand_lm.landmark[0]
    index_mcp = hand_lm.landmark[5]
    angle = math.atan2(index_mcp.y - wrist.y, index_mcp.x - wrist.x) * 180 / math.pi
    return angle

def is_swipe_confirmed(hand_lm):
    """Check for a specific hand pose to confirm swipe intent (e.g., open hand)."""
    thumb_tip = hand_lm.landmark[4]
    index_tip = hand_lm.landmark[8]
    middle_tip = hand_lm.landmark[12]
    ring_tip = hand_lm.landmark[16]
    pinky_tip = hand_lm.landmark[20]
    
    wrist = hand_lm.landmark[0]
    finger_tips = [index_tip, middle_tip, ring_tip, pinky_tip]
    extended = all(tip.y < wrist.y - 0.05 for tip in finger_tips)
    thumb_separated = abs(thumb_tip.x - index_tip.x) > 0.05
    return extended and thumb_separated

try:
    while True:
        success, img = cap.read()
        if not success:
            print("Error: Failed to read frame.")
            break

        current_time = time.time()
        frame_skip_counter += 1
        if frame_skip_counter % 3 == 0 and fps < min_fps_threshold:
            continue

        img = cv2.flip(img, 1)
        imgRGB = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        results = hands.process(imgRGB)

        h, w, _ = img.shape
        if results.multi_hand_landmarks and results.multi_handedness:
            for hand_lm, handedness in zip(results.multi_hand_landmarks, results.multi_handedness):
                hand_label = handedness.classification[0].label
                if frame_skip_counter % 2 == 0:
                    mp_draw.draw_landmarks(img, hand_lm, mp_hands.HAND_CONNECTIONS)
                
                index_tip = hand_lm.landmark[8]
                thumb_tip = hand_lm.landmark[4]
                middle_tip = hand_lm.landmark[12]
                index_x, index_y = int(index_tip.x * w), int(index_tip.y * h)
                thumb_x, thumb_y = int(thumb_tip.x * w), int(thumb_tip.y * h)
                middle_x, middle_y = int(middle_tip.x * w), int(middle_tip.y * h)
                centroid_x, centroid_y = get_hand_centroid(hand_lm, w, h)

                # Right hand: Mouse control
                if hand_label == 'Right':
                    mouse_x = np.clip(index_tip.x * virtual_width * 2.0 + virtual_min_x, virtual_min_x, virtual_min_x + virtual_width)
                    mouse_y = np.clip(index_tip.y * virtual_height * 2.0 + virtual_min_y, virtual_min_y, virtual_min_y + virtual_height)
                    dx = mouse_x - prev_x
                    dy = mouse_y - prev_y
                    speed = (dx**2 + dy**2)**0.5
                    alpha = min(0.4 + speed/500, 0.7)
                    mouse_x = alpha * mouse_x + (1 - alpha) * prev_x
                    mouse_y = alpha * mouse_y + (1 - alpha) * prev_y
                    prev_x, prev_y = mouse_x, mouse_y
                    pyautogui.moveTo(mouse_x, mouse_y, duration=0)

                    # Thumb-middle pinch for right click
                    hand_distance = estimate_hand_distance(hand_lm)
                    dynamic_middle_threshold = middle_click_distance_threshold * (0.07 / max(hand_distance, 0.01))
                    middle_distance = ((thumb_x - middle_x)**2 + (thumb_y - middle_y)**2)**0.5
                    if middle_distance < dynamic_middle_threshold and current_time - last_middle_click_time > click_debounce_time:
                        if not is_dragging:
                            pyautogui.rightClick()
                            last_middle_click_time = current_time
                            cv2.putText(img, "Right Click", (middle_x, middle_y - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2, cv2.LINE_AA)

                    # Thumb-index pinch for click and drag
                    dynamic_click_threshold = click_distance_threshold * (0.07 / max(hand_distance, 0.01))
                    distance = ((thumb_x - index_x)**2 + (thumb_y - index_y)**2)**0.5
                    if distance < dynamic_click_threshold:
                        if not is_dragging:
                            if drag_start_time == 0:
                                drag_start_time = current_time
                            elif current_time - drag_start_time >= drag_hold_time:
                                is_dragging = True
                                pyautogui.mouseDown()
                                cv2.putText(img, "Dragging", (index_x, index_y - 40), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2, cv2.LINE_AA)
                            elif current_time - last_click_time > click_debounce_time and middle_distance >= dynamic_middle_threshold:
                                pyautogui.click()
                                last_click_time = current_time
                                cv2.putText(img, "Click", (index_x, index_y - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2, cv2.LINE_AA)
                    else:
                        if is_dragging:
                            is_dragging = False
                            pyautogui.mouseUp()
                        drag_start_time = 0

                # Left hand: Horizontal swipe detection
                elif hand_label == 'Left':
                    if not is_swipe_confirmed(hand_lm):
                        swipe_history = []
                        continue
                    
                    hand_distance = estimate_hand_distance(hand_lm)
                    dynamic_swipe_threshold = swipe_distance_threshold * (0.07 / max(hand_distance, 0.01))
                    centroid_x_virtual = np.clip((centroid_x / w) * virtual_width + virtual_min_x, virtual_min_x, virtual_min_x + virtual_width)
                    centroid_y_virtual = np.clip((centroid_y / h) * virtual_height + virtual_min_y, virtual_min_y, virtual_min_y + virtual_height)
                    swipe_history.append((centroid_x_virtual, centroid_y_virtual, current_time, hand_distance))
                    swipe_history = [p for p in swipe_history if current_time - p[2] <= swipe_time_window]

                    if len(swipe_history) >= swipe_confirmation_points:
                        dx = swipe_history[-1][0] - swipe_history[0][0]
                        dy = swipe_history[-1][1] - swipe_history[0][1]
                        distance = abs(dx)
                        time_diff = swipe_history[-1][2] - swipe_history[0][2]
                        velocity = distance / max(time_diff, 0.001)
                        velocity = velocity_smoothing_factor * velocity + (1 - velocity_smoothing_factor) * prev_velocity
                        prev_velocity = velocity

                        orientation = get_hand_orientation(hand_lm)
                        is_horizontal = abs(orientation) < 85 or abs(orientation) > 95
                        is_low_vertical = abs(dy) < swipe_vertical_threshold

                        if (distance > dynamic_swipe_threshold and
                                velocity > swipe_velocity_threshold and
                                time_diff >= swipe_min_duration and
                                current_time - last_swipe_time > swipe_debounce_time and
                                is_horizontal and
                                is_low_vertical and
                                abs(dx) > abs(dy) * 2.0):
                            direction = "right" if dx > 0 else "left"
                            current_monitor = None
                            for monitor in monitors:
                                if (monitor.x <= centroid_x_virtual < monitor.x + monitor.width and
                                        monitor.y <= centroid_y_virtual < monitor.y + monitor.height):
                                    current_monitor = monitor
                                    break
                            target_monitor = get_target_monitor(centroid_x_virtual, centroid_y_virtual, direction)
                            if target_monitor and target_monitor != current_monitor:
                                move_window_to_monitor(target_monitor)
                                last_swipe_time = current_time
                                cv2.putText(img, f"Swiped {direction} to Monitor {monitors.index(target_monitor) + 1}",
                                           (index_x, index_y - 60), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 255), 2, cv2.LINE_AA)
                                arrow_start = (index_x, index_y)
                                arrow_end = (index_x + (50 if direction == "right" else -50), index_y)
                                cv2.arrowedLine(img, arrow_start, arrow_end, (255, 0, 255), 2, tipLength=0.3)
                                print(f"Swiped {direction} to monitor at x={target_monitor.x}, y={target_monitor.y}")

                # Visual feedback
                cv2.circle(img, (index_x, index_y), 8, (0, 0, 255), cv2.FILLED)
                cv2.circle(img, (thumb_x, thumb_y), 8, (0, 255, 0), cv2.FILLED)
                cv2.circle(img, (middle_x, middle_y), 8, (255, 165, 0), cv2.FILLED)
                cv2.circle(img, (int(centroid_x), int(centroid_y)), 8, (255, 0, 0), cv2.FILLED)
                cv2.putText(img, hand_label, (int(centroid_x), int(centroid_y) - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2, cv2.LINE_AA)

        cTime = time.time()
        fps = 1 / (cTime - pTime) if cTime != pTime else 0
        pTime = cTime

        cv2.putText(img, "Virtual Mouse (Horizontal Monitors)", (140, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2, cv2.LINE_AA)
        cv2.putText(img, f"FPS: {int(fps)}", (20, 60), cv2.FONT_HERSHEY_PLAIN, 1, (255, 0, 255), 1, cv2.LINE_AA)

        cv2.imshow("Virtual Mouse", img)
        if cv2.waitKey(1) & 0xFF == 27:
            break

except Exception as e:
    print(f"An error occurred: {e}")

finally:
    hands.close()
    cap.release()
    cv2.destroyAllWindows()