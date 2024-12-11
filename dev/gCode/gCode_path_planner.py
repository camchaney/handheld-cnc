# Cam's code

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch
from matplotlib.legend_handler import HandlerPatch
from sklearn.cluster import KMeans

angle_thresh = 90								# (deg)
compass_gantry_length = 106			# (mm)

# CLASSES
class ArrowHandler(HandlerPatch):
	"""Custom handler to draw arrows in legend"""
	def __init__(self, angle_deg, *args, **kwargs):
		super().__init__(*args, **kwargs)
		self.angle_deg = angle_deg
		
	def create_artists(self, legend, orig_handle,
						xdescent, ydescent, width, height, fontsize, trans):
		angle_rad = np.radians(self.angle_deg)
		
		# Calculate arrow start and end points
		arrow_length = width * 0.7
		dx = - arrow_length * np.sin(angle_rad)
		dy = arrow_length * np.cos(angle_rad)
		
		# Center the arrow
		center_x = width/2 - xdescent
		center_y = height/2 - ydescent
		
		arrow = FancyArrowPatch(
			(center_x - dx/2, center_y - dy/2),
			(center_x + dx/2, center_y + dy/2),
			arrowstyle='->',
			color=orig_handle.get_color(),
			mutation_scale=15
		)
		
		return [arrow]
	
class Path:
	def __init__(self, points_df):
		"""
		points_df: DataFrame with columns ['x', 'y', 'z'] representing ordered points in the path
		"""
		self.points = points_df
		self.direction = self._determine_direction()
		self.start = self.points.iloc[0]
		self.end = self.points.iloc[-1]
		
	def _determine_direction(self):
		# Get direction based on first two points
		first_y = self.points.iloc[0].y
		second_y = self.points.iloc[1].y
		return 1 if second_y > first_y else -1
	
class Pass:
	def __init__(self, paths, direction):
		self.paths = paths  # List of Path objects
		self.direction = direction  # 1 for forward (+y), -1 for backward (-y)
		
	@property
	def all_points(self):
		return pd.concat([path.points for path in self.paths])
		
	@property
	def x_range(self):
		points = self.all_points
		return points.x.max() - points.x.min()
	
	def rotate(self, angle_degrees):
		# Convert angle to radians
		angle_rad = np.radians(angle_degrees)

		# Create rotation matrix
		rotation_matrix = np.array([
			[np.cos(angle_rad), -np.sin(angle_rad), 0],
			[np.sin(angle_rad), np.cos(angle_rad), 0],
			[0, 0, 1]
		])

		# Rotate points in each path
		for path in self.paths:
			# Convert points to numpy array for rotation
			points_array = path.points[['x', 'y', 'z']].to_numpy()

			# Apply rotation to all points
			rotated_points = np.dot(points_array, rotation_matrix.T)

			# Update the DataFrame with rotated coordinates
			path.points['x'] = rotated_points[:, 0]
			path.points['y'] = rotated_points[:, 1]
			path.points['z'] = rotated_points[:, 2]
		
	def can_add_path(self, new_path, gantry_length):
		if not self.paths:
			return True
			
		# Check gantry length constraint
		all_points = pd.concat([self.all_points, new_path.points])
		x_span = all_points.x.max() - all_points.x.min()
		if x_span > gantry_length:
			return False
			
		# Check y-direction constraint based on pass direction
		last_path = self.paths[-1]
		last_y = last_path.points.iloc[-1].y
		new_path_start_y = new_path.points.iloc[0].y
		
		if self.direction == 1:  # Moving forward
			return new_path_start_y > last_y
		else:  # Moving backward
			return new_path_start_y < last_y
	
class Cluster:
	def __init__(self,passes,angle,rotate_back=False):
		self.passes = passes
		self.angle = angle
		if rotate_back:
			self.rotate_passes(angle)

	def rotate_passes(self, angle_degrees):
		for pass_obj in self.passes:
			pass_obj.rotate(angle_degrees)

# PARSE INPUTS
# TODO: make this actually parce gcode + nc files...
def read_gcode_csv(file_path):
	df = pd.read_csv(file_path)
	new_row = pd.DataFrame([[0, 0, 0]], columns=df.columns)     # add home point to start
	df = pd.concat([new_row, df], ignore_index=True)
	return df

def read_gcode(file_path):
	# dataframe with columns: x, y, z

	return

# MATH FUNCTIONS
# geo function
def calculate_distance(p1, p2):
	return np.sqrt((p2[0] - p1[0]) ** 2 + (p2[1] - p1[1]) ** 2)

# geo function
def get_line_info(p1, p2):
	# takes in two points and calculates the angle and length of the line they make
	dx = p2[0] - p1[0]
	dy = p2[1] - p1[1]
	m = (dy) / (dx)
	b = p2[1] - (m * p2[0])

	#dist = (dx**2 + dy**2)**(1/2)
	dist = calculate_distance(p1,p2)

	if np.isnan(m):
		# if dx = dy = 0
		beta = 0
	else:
		# alpha = np.rad2deg(np.arctan(m))
		# beta = 90 - alpha
		beta = -np.rad2deg(np.arctan(1/m))           # angle from y-axis (negative to match direction convention of router)

	return [beta, dist]

def rotate_point(point, angle_degrees):
	# Convert angle to radians
	angle_rad = np.radians(angle_degrees)
	
	# Create rotation matrix
	rotation_matrix = np.array([
		[np.cos(angle_rad), -np.sin(angle_rad), 0],
		[np.sin(angle_rad), np.cos(angle_rad), 	0],
		[0,									0,									1]
	])
	
	# Rotate the point
	rotated_point = np.dot(rotation_matrix, point)
	return rotated_point

def rotate_df(df, angle_degrees):
	# Create new DataFrame for rotated points
	rotated_df = pd.DataFrame(columns=['p0', 'p1'])
	
	# Rotate each pair of points
	for i, row in df.iterrows():
		p0_rotated = rotate_point(row.p0, angle_degrees)
		p1_rotated = rotate_point(row.p1, angle_degrees)
		
		rotated_df.loc[i] = {
			'p0': p0_rotated,
			'p1': p1_rotated
		}

	return rotated_df

def process_raw_gcode(gcode_df):
	# make line segments from raw gcode points
	# determine angle of all line segments
	# determine length of all line segments
	# turn into line segments
	# TODO: might make more sense to determine angle and length after points have been made into segments
	# TODO: delete travel moves

	segments_df = pd.DataFrame(columns=["p0","p1", "angle_d", "length"])

	# segment_df["index"] = 0
	# segments_df["p0"] = []
	# segments_df["p1"] = []
	segments_df["angle_d"] = 0
	segments_df["length"] = 0

	row = []
	index = 0

	cut_depth = min(gcode_df.z)

	for i in range(1, len(gcode_df)):
		print("pt1 = " + str([gcode_df.x[i - 1], gcode_df.y[i - 1]]))
		print("pt2 = " + str([gcode_df.x[i], gcode_df.y[i]]))
		[angle,length] = get_line_info(
			[gcode_df.x[i-1], gcode_df.y[i-1]], [gcode_df.x[i], gcode_df.y[i]]
		)

		# don't collect travel moves
		# TODO: make sure this is accounted for in firmware
		if (gcode_df.z[i] > cut_depth):
			continue

		point0 = [gcode_df.x[i-1], gcode_df.y[i-1], gcode_df.z[i-1]]
		point1 = [gcode_df.x[i], gcode_df.y[i], gcode_df.z[i]]

		# fill out temp list
		row.append({
			"p0": point0,
			"p1": point1,
			"angle_d": angle,
			"length": length
		})

		index = index + 1

	segments_df = pd.concat([segments_df, pd.DataFrame(row)], ignore_index=True)

	return segments_df

def convert_lines_to_path(lines_df):
	# Start with first point (p0) of first line
	first_line = lines_df.iloc[0]
	points = [first_line.p0]
	
	# Add each p1 point as we go through the connected lines
	for _, line in lines_df.iterrows():
		points.append(line.p1)
	
	# Convert points list to DataFrame
	points_df = pd.DataFrame([
		{'x': p[0], 'y': p[1], 'z': p[2]} for p in points
	])
	
	return Path(points_df)

def group_lines_into_paths(lines_df):
	paths = []
	remaining_lines = lines_df.copy()
	
	while not remaining_lines.empty:
		# Start a new path with the first line
		current_path_lines = pd.DataFrame([remaining_lines.iloc[0]]).copy()
		remaining_lines = remaining_lines.drop(remaining_lines.index[0])
		
		# Get direction of this path
		first_line = current_path_lines.iloc[0]
		path_direction = 1 if first_line.p1[1] - first_line.p0[1] > 0 else -1
		
		while True:
			# Find connected lines going in same direction
			last_line = current_path_lines.iloc[-1]
			last_point = last_line.p1
			
			# Look for lines that start where our last line ended
			connected_mask = remaining_lines.apply(
				lambda row: np.allclose(row.p0, last_point) and 
							((row.p1[1] - row.p0[1] > 0) == (path_direction > 0)),
				axis=1
			)
			
			if not connected_mask.any():
				break
				
			# Add the connected line to our path
			next_line_idx = connected_mask[connected_mask].index[0]
			current_path_lines = pd.concat([
				current_path_lines, 
				pd.DataFrame([remaining_lines.loc[next_line_idx]])
			])
			remaining_lines = remaining_lines.drop(next_line_idx)
		
		# Convert the connected lines into a path of points
		paths.append(convert_lines_to_path(current_path_lines))
	
	return paths

def sort_paths_into_passes(paths, gantry_length):
	remaining_paths = paths.copy()
	passes = []
	current_direction = 1  # Start moving forward
	
	while remaining_paths:
		# Start a new pass
		current_pass = Pass([], current_direction)
		
		# Find starting path based on direction and current position
		min_distance = float('inf')
		closest_path_idx = None
		
		if not passes:
			# First pass: start from path closest to origin
			for i, path in enumerate(remaining_paths):
				start_point = path.points.iloc[0]
				distance = np.sqrt(start_point.x**2 + start_point.y**2)
				if distance < min_distance:
					min_distance = distance
					closest_path_idx = i
		else:
			# Subsequent passes: start from path closest to last point of previous pass
			last_point = passes[-1].paths[-1].points.iloc[-1]
			for i, path in enumerate(remaining_paths):
				start_point = path.points.iloc[0]
				distance = np.sqrt(
					(start_point.x - last_point.x)**2 + 
					(start_point.y - last_point.y)**2 + 
					(start_point.z - last_point.z)**2
				)
				if distance < min_distance:
					min_distance = distance
					closest_path_idx = i
		
		# Add the closest path to start the pass
		current_pass.paths.append(remaining_paths.pop(closest_path_idx))
		
		# Try to add more paths to this pass
		added_path = True
		while added_path:
			added_path = False
			
			for i, path in enumerate(remaining_paths):
				if current_pass.can_add_path(path, gantry_length):
					current_pass.paths.append(remaining_paths.pop(i))
					added_path = True
					break
		
		passes.append(current_pass)
		
		# Switch direction for next pass
		current_direction *= -1
	
	return passes

# PLOTTING
def plot_segments(gcode_df, filtered_segments):
	plt.figure(figsize=(12, 6))

	for segment in filtered_segments:
		segment_points = gcode_df.iloc[segment]
		x = segment_points["x"]
		y = segment_points["y"]

		plt.scatter(x, y, label="Segment Points")

		# Optionally plot lines connecting points in the segment
		plt.plot(x, y, linestyle="-", marker="o", label="Segment Path")

	plt.xlabel("X Coordinate")
	plt.ylabel("Y Coordinate")
	plt.title("Filtered Segments")
	plt.legend()
	plt.grid(True)
	plt.show()

def plot_kmeans(plot_df, cluster_centers):
	plt.figure(figsize=(8, 8))
	
	# Plot points colored by cluster using adjusted coordinates
	color_map = plt.get_cmap('tab10')
	scatter = plt.scatter(plot_df['x'], plot_df['y'], 
											c=color_map(plot_df['cluster_label']), 
											alpha=0.03, 
											s=100)
	
	plt.scatter(cluster_centers[:, 0], cluster_centers[:, 1], 
							c='red', marker='x', s=200, linewidths=3, 
							label='Cluster Centers')
	
	# Add circle for reference
	circle = plt.Circle((0, 0), 1, fill=False, linestyle='--', color='gray')
	plt.gca().add_artist(circle)
	
	# Equal aspect ratio to maintain circular appearance
	plt.axis('equal')
	plt.grid(True)
	plt.xlabel('cos(angle)')
	plt.ylabel('sin(angle)')
	plt.title(f'K-means Clustering Results (k={len(cluster_centers)})')
	plt.legend()
	
	# Add colorbar
	# plt.colorbar(scatter, label='Cluster Label')
	
	plt.show()

def plot_rotated(grouped_dfs):
	plt.figure(figsize=(10, 6))
	cmap = plt.get_cmap('tab10')

	for idx, (cluster_label, group_data) in enumerate(grouped_dfs.items()):
		c = cmap(idx)
		df = group_data['df']
		cluster_angle = group_data['cluster_angle']

		# Rotate cluster by its cluster angle
		rotated_df = rotate_df(df.copy(), -cluster_angle)
		for i, segment in rotated_df.iterrows():
			x = [segment.p0[0], segment.p1[0]]
			y = [segment.p0[1], segment.p1[1]]
			plt.plot(x, y, color=c, linestyle='--', label=f'Rotated Cluster {cluster_label}' if i == 0 else "")

	plt.grid(True)
	plt.axis('equal')
	plt.legend()
	plt.title('Original vs Rotated Clusters')
	plt.show()

def plot_segment_by_group(grouped_dfs):
	plt.figure(figsize=(10, 6))

	cmap = plt.get_cmap('tab10')
	handles = []
	labels = []
	handler_map = {}

	for idx, (cluster_label, group_data) in enumerate(grouped_dfs.items()):
		c = cmap(idx)
		df = group_data['df']
		cluster_angle = group_data['cluster_angle']

		for i, segment in df.iterrows():
			x = [segment.p0[0], segment.p1[0]]
			y = [segment.p0[1], segment.p1[1]]
			line = plt.plot(x, y, color=c)[0]

			if i == df.index[0]:
				handles.append(line)				# TODO: what's happening here?
				labels.append(f'Cluster {cluster_label} at {cluster_angle:.1f}°')
				# Create handler for this cluster
				handler_map[line] = ArrowHandler(cluster_angle)


	plt.xlabel("X-axis")
	plt.ylabel("Y-axis")
	plt.title("Plot of Segments by Angle (wrt. Y-axis")
	plt.grid(True)
	
	# Create legend with custom handler
	plt.legend(handles, labels,
				handler_map=handler_map,
				loc='center left', 
				bbox_to_anchor=(1, 0.5))
	
	# Adjust layout to prevent legend from being cut off
	plt.tight_layout()
	plt.subplots_adjust(right=0.8)
	
	plt.show()

def plot_paths(paths):
	plt.figure(figsize=(10, 8))
	
	# Color map for different paths
	cmap = plt.get_cmap('tab10')
	
	# Plot each path in a different color
	for path_idx, path in enumerate(paths):
		c = cmap(path_idx % 10)  # Cycle through colors if more than 10 paths
		points = path.points
		plt.plot(points.x, points.y, '-', color=c, label=f'Path {path_idx}')
	
	plt.grid(True)
	plt.axis('equal')
	plt.legend()
	plt.title('Paths')
	plt.xlabel('X')
	plt.ylabel('Y')
	plt.show()

def plot_clusters(clusters):
	# Calculate number of rows and columns needed for subplots
	n_clusters = len(clusters)
	n_cols = min(3, n_clusters)  # Max 3 columns
	n_rows = (n_clusters + n_cols - 1) // n_cols
	
	# Create figure
	fig, axs = plt.subplots(n_rows, n_cols, figsize=(5*n_cols, 5*n_rows))
	if n_rows == 1 and n_cols == 1:
		axs = np.array([axs])
	axs = axs.flatten()
	
	# Color map for passes
	cmap = plt.get_cmap('tab10')
	
	# Plot each cluster in a subplot
	for cluster_idx, cluster in enumerate(clusters):
		ax = axs[cluster_idx]
		
		# Plot each pass in the cluster with different colors
		for pass_idx, pass_obj in enumerate(cluster.passes):
			c = cmap(pass_idx % 10)  # Cycle through colors if more than 10 passes
			
			# Plot all points in the pass
			points = pass_obj.all_points
			ax.plot(points.x, points.y, 'o-', color=c, 
					label=f'Pass {pass_idx} ({"+y" if pass_obj.direction > 0 else "-y"})')
		
		ax.set_title(f'Cluster {cluster_idx} (angle: {cluster.angle:.1f}°)')
		ax.grid(True)
		ax.axis('equal')
		ax.legend()
	
	# Remove empty subplots if any
	for idx in range(cluster_idx + 1, len(axs)):
		fig.delaxes(axs[idx])
	
	plt.tight_layout()
	plt.show()

def group_segments(segments_df):
	# angle_range = 90     # this range is 2x the range of one side of y-axis

	# start with angle 0
	group_angle = 0

	k = 1           # can they all fit in one bucket?
	buckets_valid = 0

	while (not buckets_valid):
		kmeans_df = segments_df
		# We want to plot the angles of every line in our design on the unit circle.
		# Note: remember that the angles were calculated w.r.t. the y-axis, so the x and y coordinates should be calculated as such
		kmeans_df['x'] = np.sin(np.radians(kmeans_df.angle_d))
		kmeans_df['y'] = np.cos(np.radians(kmeans_df.angle_d))

		kmeans = KMeans(n_clusters=k, random_state=0).fit(kmeans_df[['x', 'y']])
		kmeans_df['cluster_label'] = kmeans.labels_

		cluster_centers = kmeans.cluster_centers_
		cluster_center_angles = np.degrees(np.arctan2(cluster_centers[:, 0], cluster_centers[:, 1]))
		# cluster_center_angles = cluster_center_angles % 180
		kmeans_df['cluster_angle'] = kmeans_df['cluster_label'].map(
			dict(enumerate(cluster_center_angles)))

		buckets_valid = 1

		group_dfs = {}

		for cluster_label in np.unique(kmeans_df['cluster_label']):
			# Extract the angles in degrees for the current cluster
			cluster_angles = kmeans_df[kmeans_df['cluster_label'] == cluster_label]['angle_d']

			# Compute the min and max angles within the cluster
			min_angle = cluster_angles.min()
			max_angle = cluster_angles.max()

			# Handle wraparound at 360 degrees by considering both directions
			angular_range = min((max_angle - min_angle), (min_angle + 360 - max_angle))

			# Reorganize dataframe into dict (one dict containing dataframes for each cluster)
			cluster_df = pd.DataFrame({
				'p0': kmeans_df[kmeans_df['cluster_label'] == cluster_label]['p0'],
				'p1': kmeans_df[kmeans_df['cluster_label'] == cluster_label]['p1'],
				'angle_d': kmeans_df[kmeans_df['cluster_label'] == cluster_label]['angle_d'],
			})
			group_dfs[cluster_label] = {
				'df': cluster_df,
				'cluster_angle': kmeans_df[kmeans_df['cluster_label'] == cluster_label]['cluster_angle'].iloc[0]
			}

			# Check if the angular range exceeds the boundary
			if angular_range > angle_thresh:
				k = k + 1
				buckets_valid = 0
				break

	# Visualize stuff
	plot_kmeans(kmeans_df.copy(), cluster_centers)

	return group_dfs

def main():
	# my_df = read_gcode_csv("dev/gCode/basePlate_test.csv")
	my_df = read_gcode_csv("dev/gCode/cal.csv")
	# my_df = read_gcode("dev/gCode/basePlate_test.nc")
	plt.scatter(my_df.x, my_df.y)
	plt.grid()
	plt.title("Plotting whole GCODE Path (just the points)")
	plt.show()

	gcode_df_processed = process_raw_gcode(my_df)
	grouped_segments_df = group_segments(gcode_df_processed)
	plot_rotated(grouped_segments_df)
	plot_segment_by_group(grouped_segments_df)

	# Sort clustered dataframes
	clusters = []
	for idx, (cluster_label, group_data) in enumerate(grouped_segments_df.items()):
		df = group_data['df']
		cluster_angle = group_data['cluster_angle']

		# Rotate cluster by its cluster angle
		rotated_df = rotate_df(df.copy(), -cluster_angle)
		# Group cluster into paths
		paths = group_lines_into_paths(rotated_df)
		plot_paths(paths)
		passes = sort_paths_into_passes(paths, compass_gantry_length)
		clusters.append(Cluster(passes,cluster_angle,rotate_back=True))
	
	plot_clusters(clusters)


if __name__ == "__main__":
	main()
