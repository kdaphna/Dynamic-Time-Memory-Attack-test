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
        print("am i here!")
        for filename in filenames:
            if regex.match(filename):
                files.append(os.path.join(root, filename))
    return files
   
def read_array(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    return [float(num) for num in content.split()]
   
 
def combine_arrays(*arrays):
    # Find the length of the longest array
    max_length = max(len(arr) for arr in arrays)
    
    # Initialize the result array with zeros
    result = [0] * max_length
    
    # Sum the values at each index
    for arr in arrays:
        for i in range(len(arr)):
            print(arr[i])
            result[i] += arr[i]/len(arr)
    
    return result 
   
   
# Function to read array from file
def read_array_from_file(success):
    success_vector = []
    if(success):
        files = search_files('/home/daphna/TMDTO/codeDaphnaThesis/data', r'successTime\d+')
        for file_path in files:
            array = read_array(file_path)
            success_vector.append(array)
        return combine_arrays(*success_vector)
    else:
        files = search_files('/home/daphna/TMDTO/codeDaphnaThesis/data', r'failTime\d+')
        for file_path in files:
            array = read_array(file_path)
            success_vector.append(array)
        return combine_arrays(*success_vector)

# Function to calculate the sum of the array
def calculate_sum(array):
    array = combine_arrays(array)
    return sum(array)/len(array)

# Function to create a graph
def create_graph(array, success):
    plt.figure(figsize=(10, 5))
    plt.plot(array, marker='o', linestyle='-', color='b', label='Success rate of Dynamic Hellman Tables')
    plt.xlabel('Sets of Dynamic Searches performed (each set is 100 Dynamic Searches)')
    plt.ylabel('Success rate')
    plt.title('The success rate increases over runs')
    plt.legend()
    plt.grid(True)
    if(success):
        plt.savefig('SuccessTimeGraph.png')  # Save the plot to a file
        plt.close()  # Close the plot to free up memory
    else:
        plt.savefig('FailTimeGraph.png')  # Save the plot to a file
        plt.close()  # Close the plot to free up memory

    
def create_linear_graph(array, success):
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
    formatted_slope = f"{slope:.2f}"
    formatted_intercept = f"{intercept:.2f}"
    plt.plot(x, regression_line, color='red', label=f'Regression line: y = {formatted_slope}x + {formatted_intercept}')

    # Add the formula to the plot
    formula_text = f'R² = {r_squared:.2f}\np-value = {p_value:.4f}'
    plt.text(0.05, 0.85, formula_text, transform=plt.gca().transAxes,
             fontsize=10, bbox=dict(facecolor='white', alpha=0.5))

    # Add labels and legend
    plt.xlabel('Sets of Dynamic Searches performed (each set is 100 Dynamic Searches)')
    plt.ylabel('Mean Duration of an Online Search (seconds)')
    #plt.title('Simple Linear Regression')
    #plt.legend()

    # Show the plot
    if(success):
        plt.title('The time complexity of a successful search')
        plt.legend(loc='lower left')
        plt.savefig('SuccessTimeGraphLinear0802.png')  # Save the plot to a file
        plt.close()  # Close the plot to free up memory
    else:
        plt.title('The time complexity of a failed search')
        plt.legend(loc='lower left')
        plt.savefig('FailTimeGraphLinear0802.png')  # Save the plot to a file
        plt.close()  # Close the plot to free up memory

   

# Main function
def main():
    arraySuccess = read_array_from_file(1)
    arrayFail = read_array_from_file(0)

    create_graph(arraySuccess, 1)
    create_linear_graph(arraySuccess, 1)
    create_graph(arrayFail, 0)
    create_linear_graph(arrayFail, 0)

# Run the main function
if __name__ == "__main__":
    main()
    
    
    


