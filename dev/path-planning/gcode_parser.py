import re
import pandas as pd
from gcodeparser import GcodeParser

# TODO: only works for linear moves. If we move forward with the gCode method, we would either need our own
#   post processor that works in only linear moves, or figure out how to convert arc moves to cartesian coordinates
# Function to parse a single line and extract X, Y, Z coordinates
def parse_line(line):
  # Initialize coordinates with None
  x, y, z = None, None, None
  
  # Regular expressions to find X, Y, Z coordinates
  x_match = re.search(r'X([-+]?\d*\.?\d+)', line)
  y_match = re.search(r'Y([-+]?\d*\.?\d+)', line)
  z_match = re.search(r'Z([-+]?\d*\.?\d+)', line)
  
  if x_match:
    x = float(x_match.group(1))
  if y_match:
    y = float(y_match.group(1))
  if z_match:
    z = float(z_match.group(1))
  
  return x, y, z

# Function to parse the .nc file and create a DataFrame
def parse_nc_file(file_path):
  coordinates = {'x': [], 'y': [], 'z': []}
  
  with open(file_path, 'r') as file:
    for line in file:
      x, y, z = parse_line(line)
      if x is not None or y is not None or z is not None:
        coordinates['x'].append(x)
        coordinates['y'].append(y)
        coordinates['z'].append(z)
    
  df = pd.DataFrame(coordinates)
  return df

# Usage
file_path = 'handheld-cnc/dev/gCode/basePlate_test.nc'
df = parse_nc_file(file_path)
print(df)
