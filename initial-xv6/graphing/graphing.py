import matplotlib.pyplot as plt
import pandas as pd
from collections import defaultdict

# Step 1: Read and filter the data
def read_and_filter_data(input_file, output_file):
    with open(input_file, 'r') as file:
        lines = file.readlines()

    # Function to check if the second and third elements match
    def elements_match(line1, line2):
        return line1.split(',')[1:] == line2.split(',')[1:]

    # Filter lines based on the condition
    filtered_lines = []
    n = len(lines)

    for i in range(n):
        if i == 0 or i == n - 1:
            filtered_lines.append(lines[i])
        else:
            prev_line = lines[i - 1]
            curr_line = lines[i]
            next_line = lines[i + 1]
            if not (elements_match(prev_line, curr_line) and elements_match(curr_line, next_line)):
                filtered_lines.append(curr_line)

    # Write the filtered content to a new file
    with open(output_file, 'w') as file:
        file.writelines(filtered_lines)

# Step 2: Further filter the data to keep only records with the first value in range 455000 to 459000
def filter_data_range(input_file, output_file, start_range, end_range):
    with open(input_file, 'r') as file:
        lines = file.readlines()

    filtered_lines = [line for line in lines if start_range <= int(line.split(',')[0]) <= end_range]

    with open(output_file, 'w') as file:
        file.writelines(filtered_lines)

# Step 3: Plot the data and save the plot
def plot_data(file, output_image):
    data = pd.read_csv(file, header=None, names=['Time', 'PID', 'Priority'])

    # Sort data by time
    data.sort_values(by='Time', inplace=True)

    # Get unique process IDs
    pids = data['PID'].unique()

    # Create a color map
    color_map = plt.cm.get_cmap('tab10')
    colors = {pid: color_map(i / len(pids)) for i, pid in enumerate(pids)}

    # Create the plot
    plt.figure(figsize=(15, 10))

    for pid in pids:
        pid_data = data[data['PID'] == pid]
        plt.plot(pid_data['Time'], pid_data['Priority'], color=colors[pid], label=f'Process {pid}', linewidth=2)

    plt.xlabel('Time')
    plt.ylabel('Priority Queue')
    plt.title('MLFQ Scheduler Process Priority over Time using schedulertest')
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.grid(True, which='both', linestyle='--', linewidth=0.5)
    plt.ylim(-0.5, 3.5)
    plt.yticks(range(4))

    plt.tight_layout()
    plt.savefig(output_image)  # Save the plot as an image file
    plt.close()  # Close the plot to free up memory

# File paths
input_file = 'initial-xv6/graphing/data.txt'
filtered_file = 'initial-xv6/graphing/filtered_data.txt'
filtered_data_limited_file = 'initial-xv6/graphing/filtered_data_limited.txt'
output_image = 'initial-xv6/graphing/plot.png'

# Execute steps
read_and_filter_data(input_file, filtered_file)
filter_data_range(filtered_file, filtered_data_limited_file, 0, 99999)
plot_data(filtered_data_limited_file, output_image)