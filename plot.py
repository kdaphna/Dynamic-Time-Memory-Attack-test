import os
import re
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import linregress

def search_files(directory, pattern):
    files = []
    regex = re.compile(pattern)
    print("regex,", regex)
    for root, _, filenames in os.walk(directory):
        for filename in filenames:
            if regex.match(filename):
                files.append(os.path.join(root, filename))
    return files
   
def read_array(file_path):
    #with open(file_path, 'r') as file:
    #content = file.read()
    #elements = content.split()
    #numbers = []
    #for element in elements:
    #   element = element.strip()  # Remove any leading/trailing whitespace
    #    if element.isdigit():  # Check if the element is a valid number
    #        numbers.append(int(element))
    #    else:
    #        print(f"Skipping invalid element: {element}")
    #return numbers
    with open(file_path, 'r') as file:
        content = file.read()
    return [int(num) for num in content.split()]
    
   
# Function to read array from file
def read_array_from_file():
    success_vector = []
    files = search_files('/home/daphna/TMDTO/codeDaphnaThesis/data', r'successArr\d+')
    for file_path in files:
        array = read_array(file_path)
        success_vector.append(calculate_sum(array))
    return success_vector

# Function to calculate the sum of the array
def calculate_sum(array):
    return sum(array)/1000

# Function to create a graph
def create_graph(array):
    plt.figure(figsize=(10, 5))
    plt.plot(array, marker='o', linestyle='-', color='b', label='Success Rate of Dynamic Hellman Tables')
    plt.xlabel('Number of runs')
    plt.ylabel('Success rate')
    plt.title('The success rate increases over runs')
    plt.legend()
    plt.grid(True)
    plt.savefig('SuccessGraph0402.png')  # Save the plot to a file
    plt.close()  # Close the plot to free up memory
    
def create_linear_graph(array):
    # Sample data
    y = np.array([float(i) for i in array])
    size = len(array)
    x = array = np.arange(1, size + 1)
    # Perform linear regression
    slope, intercept, r_value, p_value, std_err = linregress(x, y)

    # Calculate R-squared
    r_squared = r_value**2

    # Print regression statistics
    print(f"Slope: {slope}")
    print(f"Intercept: {intercept}")
    print(f"R-squared: {r_squared}")
    print(f"Correlation co-efficient: {r_value}")
    print(f"P-value: {p_value}")
    print(f"Standard Error: {std_err}")

    # Plot the data points
    plt.scatter(x, y, color='blue', label='Data points')

    # Plot the regression line
    regression_line = slope * x + intercept
    # Format the slope and intercept to two decimal places
    formatted_slope = f"{slope:.2f}"
    formatted_intercept = f"{intercept:.2f}"
    plt.plot(x, regression_line, color='red', label=f'Regression line: y = {formatted_slope}x + {formatted_intercept}')

    # Add the formula to the plot
    #formula_text = f'y = {slope:.2f}x + {intercept:.2f}\nR² = {r_squared:.2f}'
    formula_text = f'R² = {r_squared:.2f}\np-value = {p_value:.4f}'
    plt.text(0.05, 0.85, formula_text, transform=plt.gca().transAxes,
             fontsize=10, bbox=dict(facecolor='white', alpha=0.5))

    # Add labels and legend
    plt.xlabel('Sets of Dynamic Searches performed (each set is 100 Dynamic Searches)')
    plt.ylabel('Success Rate')
    plt.title('Success Rate of Dynamic Hellman Tables')
    plt.legend(loc='lower left')

    # Show the plot
    plt.savefig('SuccessGraphLinear0802.png')  # Save the plot to a file
    plt.close()  # Close the plot to free up memory
   

# Main function
def main():
    array = read_array_from_file()
    create_graph(array)
    create_linear_graph(array)

# Run the main function
if __name__ == "__main__":
    main()
    
    
    


