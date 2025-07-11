import struct
import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from enum import IntEnum
import mplcursors

# Constants to match your firmware
PACKET_START = 0xAA
PACKET_END = 0x55
PACKET_HEADER = 0xA0
PACKET_PATH_POINT = 0xA3
PACKET_SENSORS = 0x01
PACKET_AUX = 0x02
MAX_STRING_LENGTH = 32

"""NOTE: The following decoder is based on the following firmware version"""
working_version = "0.3.1"

num_sensors = 4

class CutState(IntEnum):
	NOT_CUT_READY = 0
	NOT_USER_READY = 1
	CUT_READY = 2
	CUTTING = 3
	PLUNGING = 4
	RETRACTING = 5

class BinaryLogDecoder:
	def __init__(self, filename):
		self.filename = filename
		self.design_info = {}
		self.points = []
		self.sensor_data = []
		self.aux_data = []
		
	def decode_file(self):
		"""Process the entire binary log file."""
		with open(self.filename, 'rb') as f:
			# First try to read the file header
			self._decode_header(f)
			
			# Read data packets until EOF
			while True:
				try:
					packet_type = f.read(1)
					if not packet_type:  # EOF
						break
						
					# Rewind one byte so we can process the packet type in the appropriate function
					f.seek(-1, os.SEEK_CUR)
					byte_index = f.tell()

					# Process based on packet type
					packet_type_val = int.from_bytes(packet_type, byteorder='little')

					if packet_type_val == PACKET_PATH_POINT:
						self._decode_path_point(f)
					elif packet_type_val == PACKET_START:
						# Read the next byte to determine real packet type
						start_marker = f.read(1)[0]  # Skip start marker
						next_type = f.read(1)[0]  # Read real packet type
						
						if next_type == PACKET_SENSORS:
							self._decode_sensor_packet(f)
						elif next_type == PACKET_AUX:
							self._decode_aux_packet(f)
						else:
							byte_index = f.tell()
							print(f"{byte_index} - Unknown packet type after start marker: {next_type}")
							# Skip to next start marker
							# self._find_next_start_marker(f)
							f.read(1)  # Skip one byte to avoid infinite loop
					else:
						byte_index = f.tell()
						print(f"{byte_index} - Unknown packet type: {packet_type_val}")
						# Skip to next start marker
						# self._find_next_start_marker(f)
						f.read(1)  # Skip one byte to avoid infinite loop
						
				except EOFError:
					break
				except Exception as e:
					print(f"Error decoding file: {e}")
					# Try to recover and continue
					# self._find_next_start_marker(f)
					f.read(1)
	
	def _find_next_start_marker(self, f):
		"""Find the next start marker in the file."""
		while True:
			try:
				byte = f.read(1)
				if not byte:  # EOF
					raise EOFError
				if byte[0] == PACKET_START:
					# Rewind so the marker can be processed
					f.seek(-1, os.SEEK_CUR)
					return
			except EOFError:
				raise
			
	def _decode_header(self, f):
		"""Decode the file header."""
		try:
			packet_type = int.from_bytes(f.read(1), byteorder='little')
			if packet_type != PACKET_HEADER:
				print(f"Warning: Expected header packet (0xA0), got {packet_type}")
				return
				
			firmware_version = f.read(MAX_STRING_LENGTH).decode('utf-8').rstrip('\x00')
			if firmware_version != working_version:
				raise ValueError(f"Firmware version mismatch: expected {working_version}, got {firmware_version}")
			design_name = f.read(MAX_STRING_LENGTH).decode('utf-8').rstrip('\x00')
			f.read(3)	 # Skip 3 bytes (padding)
			cal_params = []
			for i in range(num_sensors):
				(cx, cy, cr) = struct.unpack('fff', f.read(12))
				cal_params.append({
					'cx': cx,
					'cy': cy,
					'cr': cr
				})
			num_points = struct.unpack('<H', f.read(2))[0]  # uint16_t
			
			self.design_info = {
				'firmware_version': firmware_version,
				'design_name': design_name,
				'cal_params': cal_params,
				'num_points': num_points
			}

			f.read(2)
			
			print(f"File Header: {self.design_info}")
			print("byte index after header:", f.tell())
			
		except Exception as e:
			print(f"Error decoding header: {e}")
	
	def _decode_path_point(self, f):
		"""Decode a path point."""
		try:
			packet_type = int.from_bytes(f.read(1), byteorder='little')
			f.read(1)
			point_index = struct.unpack('<H', f.read(2))[0]  # uint16_t
			# f.read(2)
			# Read point coordinates
			(x,y,z) = struct.unpack('fff', f.read(12))
			feature = int.from_bytes(f.read(1), byteorder='little')
			f.read(3)
			
			point = {
				'point_index': point_index,
				'x': x,
				'y': y,
				'z': z,
				'feature': feature
			}
			
			self.points.append(point)
					
		except Exception as e:
			print(f"Error decoding path point: {e}")
	
	def _decode_sensor_packet(self, f):
		"""Decode a sensor data packet."""
		try:
			# We've already read packetType in the caller
			# f.read(1)  # Skip the packet type byte (why tho?)
			# f.read(1)  # Skip the padding byte (why tho?)
			byte_index = f.tell()
			# print("entering sensor at ", byte_index)
			f.read(2)
			time = struct.unpack('I', f.read(4))[0]  # uint32_t
			#f.read(1)
			
			sensors = []
			for i in range(num_sensors):
				#TODO: make this work for the raw int and byte sensor data instead
				(dx, dy, onSurface) = struct.unpack('ff?', f.read(9))  # float x, y, onSurface (bool)
				sq = int.from_bytes(f.read(1), byteorder='little')
				rawDataSum = int.from_bytes(f.read(1), byteorder='little')
				f.read(1)
				
				sensors.append({
					'dx': dx,
					'dy': dy,
					'onSurface': onSurface,
					'sq': sq,
					'rawDataSum': rawDataSum
				})
			
			dt = struct.unpack('I', f.read(4))[0]  # uint32_t
			#f.read(4)
			
			# Read end marker
			end_marker = int.from_bytes(f.read(1), byteorder='little')
			if end_marker != PACKET_END:
				byte_index = f.tell()
				print(f"{byte_index} - Warning: Expected end marker (0x55), got {end_marker}")

			f.read(3)
			
			self.sensor_data.append({
				'time': time,
				'sensors': sensors,
				'dt': dt
			})
			
		except Exception as e:
			print(f"Error decoding sensor packet: {e}")
	
	def _decode_aux_packet(self, f):
		"""Decode an auxiliary data packet."""
		try:
			# We've already read packetType in the caller
			# byte_index = f.tell()
			# print("entering aux at ", byte_index)
			f.read(2)
			time = struct.unpack('I', f.read(4))[0]  # uint32_t
			(pose_x, pose_y, pose_yaw) = struct.unpack('fff', f.read(12))		# float x, y, z
			curr_point_index = struct.unpack('<H', f.read(2))[0]				# uint16_t
			# print("...after reading pose and curr_point_index", f.tell())
			f.read(2)
			(goal_x, goal_y, goal_z, fr) = struct.unpack('ffff', f.read(16))	# float x, y, z, fr
			feature = int.from_bytes(f.read(1), byteorder='little')
			f.read(3)
			(tool_x, tool_y, tool_z) = struct.unpack('fff', f.read(12))			# float x, y, z
			(des_x, des_y, des_z) = struct.unpack('fff', f.read(12))			# float x, y, z
			cut_state = int.from_bytes(f.read(1), byteorder='little')			# int8_t
			# f.read(1)
			# f.read(1)
			
			end_marker = int.from_bytes(f.read(1), byteorder='little')
			if end_marker != PACKET_END:
				byte_index = f.tell()
				print(f"{byte_index} - Warning: Expected aux end marker (0x55), got {end_marker}")
			f.read(2)
			
			self.aux_data.append({
				'time': time,
				'pose': {
					'x': pose_x,
					'y': pose_y,
					'yaw': pose_yaw
				},
				'curr_point_index': curr_point_index,
				'goal': {
					'x': goal_x,
					'y': goal_y,
					'z': goal_z,
					'fr': fr,
					'feature': feature
				},
				'tool_pos': {
					'x': tool_x,
					'y': tool_y,
					'z': tool_z
				},
				'des_pos': {
					'x': des_x,
					'y': des_y,
					'z': des_z
				},
				'cut_state': CutState(cut_state).name if cut_state < len(CutState) else f"UNKNOWN({cut_state})"
			})

			if (curr_point_index > 10):
				blah = 1
			
		except Exception as e:
			print(f"Error decoding aux packet: {e}")
	
	def get_design_dataframe(self):
		"""Convert design points to a pandas DataFrame."""
		if not self.points:
			return pd.DataFrame()
			
		return pd.DataFrame(self.points)
	
	def get_sensor_dataframe(self):
		"""Convert sensor data to a pandas DataFrame."""
		if not self.sensor_data:
			return pd.DataFrame()
		
		# Flatten the sensor data
		flattened_data = []
		for data_point in self.sensor_data:
			time = data_point['time']
			dt = data_point['dt']
			for i, sensor in enumerate(data_point['sensors']):
				flattened_data.append({
					'time': time,
					'dt': dt,
					'sensor_id': i,
					'dx': sensor['dx'],
					'dy': sensor['dy'],
					'onSurface': sensor['onSurface'],
					'sq': sensor['sq'],
					'rawDataSum': sensor['rawDataSum']
				})
		
		return pd.DataFrame(flattened_data)
	
	def get_aux_dataframe(self):
		"""Convert auxiliary data to a pandas DataFrame."""
		if not self.aux_data:
			return pd.DataFrame()
			
		# Flatten the auxiliary data
		flattened_data = []
		for data in self.aux_data:
			flattened_data.append({
				'time': data['time'],
				'pose_x': data['pose']['x'],
				'pose_y': data['pose']['y'],
				'pose_yaw': data['pose']['yaw'],
				'curr_point_index': data['curr_point_index'],
				'goal_x': data['goal']['x'],
				'goal_y': data['goal']['y'],
				'goal_z': data['goal']['z'],
				'tool_pos': data['tool_pos'],
				'des_pos': data['des_pos'],
				'cut_state': data['cut_state']
			})
		
		return pd.DataFrame(flattened_data)
	
	def process_sensor_data(self):
		# TODO: add inputs for modifying sensor attributes
		#	- sensor offsets
		#	- sensor calibration
		#	- low pass filter
		#	- sensor fusion
		"""Process raw sensor data according to firmware logic."""
		df = self.get_sensor_dataframe()
		df_aux = self.get_aux_dataframe()
		if df.empty:
			print("No sensor data to process")
			return pd.DataFrame()
			
		# Constants
		ns = 4		# number of sensors
		lx = 120	# spacing between sensors in x
		ly = 140	# spacing between sensors in y

		processed_data = []
		last_pose = {'x': 0.0, 'y': 0.0, 'yaw': 0.0}  # Initial pose
		cal_params = self.design_info['cal_params']
		# Interpolate tool_pos for each dimension
		aux_times = df_aux['time'].to_numpy()
		tool_pos_interp = {}
		for dim in ['x', 'y', 'z']:
			tool_pos_interp[dim] = np.interp(
				df['time'].unique(),
				aux_times,
				df_aux['tool_pos'].apply(lambda d: d[dim]).to_numpy()
			)

		sensor_offsets = np.array([
			[lx, lx, -lx, -lx],     # x offsets
			[-ly, ly, -ly, ly]      # y offsets
		]) * 0.5
		
		# Process each unique timestamp
		for ind, time in enumerate(df['time'].unique()):
			time_df = df[df['time'] == time]
			dt = time_df['dt'].iloc[0]				# dt is same for all sensors at this timestamp
			tool_pos = {dim: tool_pos_interp[dim][ind] for dim in ['x', 'y', 'z']}
			
			# Arrays to store measurements
			meas_vel = [[0.0] * 4, [0.0] * 4]		# 2x4 array for x,y velocities
			est_ang_vel = [0.0] * 8					# 8 angular velocity calculations
			est_vel = [[0.0] * ns, [0.0] * ns]		# Estimated velocities after rotation
			
			# Process each sensor's data
			for i, row in time_df.iterrows():
				sensor_id = int(row['sensor_id'])
				dx = row['dx']  # Negative convention used to flip sensor's z axis
				dy = row['dy']
				sq = row['sq']
				onSurface = row['onSurface']
				
				# Apply calibration if surface quality is good enough
				# if sq > 20:
				if onSurface:
					cal = cal_params[sensor_id]
					meas_vel[0][sensor_id] = cal['cx'] * (dx*np.cos(cal['cr']) - dy*np.sin(cal['cr'])) / dt
					meas_vel[1][sensor_id] = cal['cy'] * (dx*np.sin(cal['cr']) + dy*np.cos(cal['cr'])) / dt
				else:
					meas_vel[0][sensor_id] = float('nan')
					meas_vel[1][sensor_id] = float('nan')
			
			V_meas = np.array(meas_vel)  # Shape will be (2,4)

			# Calculate angular velocities using same equations as firmware
			est_ang_vel[0] = (meas_vel[0][2] - meas_vel[0][0])/ly	# sensor [0 - 2 -] (comparing x velocities)
			est_ang_vel[1] = (meas_vel[0][2] - meas_vel[0][1])/ly	# sensor [- 1 2 -]
			est_ang_vel[2] = (meas_vel[0][3] - meas_vel[0][0])/ly	# sensor [0 - - 3]
			est_ang_vel[3] = (meas_vel[0][3] - meas_vel[0][1])/ly	# sensor [- 1 - 3]
			est_ang_vel[4] = (meas_vel[1][1] - meas_vel[1][0])/lx	# sensor [0 1 - -] (comparing y velocities)
			est_ang_vel[5] = (meas_vel[1][1] - meas_vel[1][2])/lx	# sensor [- 1 2 -]
			est_ang_vel[6] = (meas_vel[1][3] - meas_vel[1][0])/lx	# sensor [0 - - 3]
			est_ang_vel[7] = (meas_vel[1][3] - meas_vel[1][2])/lx	# sensor [- - 2 3]
			
			# Average angular velocities (excluding NaN values)
			valid_ang_vel = [v for v in est_ang_vel if not np.isnan(v)]
			avg_ang_vel = np.mean(valid_ang_vel) if valid_ang_vel else float('nan')
			
			# Body position estimation
			pose = last_pose.copy()
			R = np.array([
				[np.cos(pose['yaw']), -np.sin(pose['yaw'])],
				[np.sin(pose['yaw']), np.cos(pose['yaw'])]
			])
			V_body = R @ V_meas + avg_ang_vel * (R @ sensor_offsets)
			est_vel[0] = V_body[0]			# x velocities
			est_vel[1] = V_body[1]			# y velocities

			# Average linear velocities
			valid_vels = [(est_vel[0][i], est_vel[1][i]) 
						for i in range(ns) 
						if not np.isnan(est_vel[0][i]) and not np.isnan(est_vel[1][i])]
			
			if valid_vels:
				avg_vel_x = np.mean([v[0] for v in valid_vels])
				avg_vel_y = np.mean([v[1] for v in valid_vels])
				valid_sensors = True
			else:
				avg_vel_x = float('nan')
				avg_vel_y = float('nan')
				valid_sensors = False

			# Integrate to get position and orientation
			if valid_sensors:
				pose['yaw'] += avg_ang_vel * dt
				pose['x'] += avg_vel_x * dt
				pose['y'] += avg_vel_y * dt
				last_pose = pose.copy()
			
			processed_data.append({
				'time': time,
				'dt': dt,
				'pose_x': pose['x'],
				'pose_y': pose['y'],
				'pose_yaw': pose['yaw'],
				'ang_vel_array': est_ang_vel,
				'ang_vel': avg_ang_vel,
				'vel_x': avg_vel_x,
				'vel_y': avg_vel_y,
				'tool_pos_x': tool_pos['x'],
				'tool_pos_y': tool_pos['y']
			})

		return pd.DataFrame(processed_data)
	
	def plot_design(self):
		"""Plot the design points."""
		df = self.get_design_dataframe()
		if df.empty:
			print("No design points to plot")
			return
			
		plt.figure(figsize=(10, 8))

		# Plot each point
		plt.plot(df['x'], df['y'])

		plt.title('Design Points')
		plt.xlabel('X')
		plt.ylabel('Y')
		plt.legend()
		plt.grid(True)
		plt.axis('equal')
		plt.show()
	
	def plot_trajectory(self):
		"""Plot the machine trajectory."""
		df = self.get_aux_dataframe()
		df_processed = self.process_sensor_data()
		if df_processed.empty:
			print("No processed sensor data to plot")
			return
		if df.empty:
			print("No auxiliary data to plot")
			return
			
		plt.figure(figsize=(12, 10))
		
		# Plot the machine position
		ax1 = plt.subplot(1,2,1)
		ax1.plot(df['pose_x'], df['pose_y'], '-b', label='Machine Pose (Aux)')
		ax1.plot(df_processed['pose_x'], df_processed['pose_y'], '--r', alpha=0.7, label='Machine Pose (Processed)')
		
		# Highlight different cut states with different colors
		cut_states = df['cut_state'].unique()
		for state in cut_states:
			state_df = df[df['cut_state'] == state]
			ax1.scatter(state_df['pose_x'], state_df['pose_y'], label=state, alpha=0.7)
		
		# Plot the design points if available
		design_df = self.get_design_dataframe()
		if not design_df.empty:
			ax1.plot(design_df['x'], design_df['y'], '--k', alpha=0.5)

		ax1.set_title('Machine Trajectory')
		ax1.set_xlabel('X')
		ax1.set_ylabel('Y')
		ax1.legend()
		ax1.grid(True)
		ax1.axis('equal')

		ax2 = plt.subplot(2,2,2)
		ax2.plot(df['time']/1_000, df['pose_x'], label='Pose X (Aux)')
		ax2.plot(df_processed['time']/1_000, df_processed['pose_x'], '--r', alpha=0.7, label='Pose X (Processed)')
		ax2.set_ylabel('Pose X')
		ax2.set_xlabel('Time (ms)')
		ax2.legend()
		ax2.grid(True)

		ax3 = plt.subplot(2,2,4)
		ax3.plot(df['time']/1_000, df['pose_y'], label='Pose Y (Aux)')
		ax3.plot(df_processed['time']/1_000, df_processed['pose_y'], '--r', alpha=0.7, label='Pose Y (Processed)')
		ax3.set_ylabel('Pose Y')
		ax3.set_xlabel('Time (ms)')
		ax3.legend()
		ax3.grid(True)

		plt.show()
	
	def plot_sensor_data(self):
		"""Plot sensor data over time."""
		df = self.get_sensor_dataframe()
		df_processed = self.process_sensor_data()
		if df.empty:
			print("No sensor data to plot")
			return
			
		# Convert time to seconds for better readability
		df['time_sec'] = df['time'] / 1_000

		unique_sens_times = df['time_sec'].unique()
		dt_sens = pd.Series(unique_sens_times).diff()
				
		# Create subplots for dx, dy, and sq
		fig, axs = plt.subplots(5, 1, figsize=(14, 10), sharex=True)

		# Create secondary axes for time differences
		ax2_0 = axs[0].twinx()
		ax2_1 = axs[1].twinx()
		ax2_2 = axs[2].twinx()
		ax2_3 = axs[3].twinx()
		
		# Plot dx for each sensor
		for sensor_id in df['sensor_id'].unique():
			sensor_df = df[df['sensor_id'] == sensor_id]
			axs[0].plot(sensor_df['time_sec'], sensor_df['dx'], label=f'Sensor {sensor_id}')
		ax2_0.plot(unique_sens_times, dt_sens, label='Δt', color='grey', alpha=0.5)
		axs[0].set_ylabel('dx')
		ax2_0.set_ylabel('Δt (ms)', color='red')
		ax2_0.tick_params(axis='y', labelcolor='red')
		axs[0].legend(loc='upper left')
		ax2_0.legend(loc='upper right')
		axs[0].grid(True)
		
		# Plot dy for each sensor
		for sensor_id in df['sensor_id'].unique():
			sensor_df = df[df['sensor_id'] == sensor_id]
			axs[1].plot(sensor_df['time_sec'], sensor_df['dy'], label=f'Sensor {sensor_id}')
		ax2_1.plot(unique_sens_times, dt_sens, label='Δt', color='grey', alpha=0.5)
		axs[1].set_ylabel('dy')
		ax2_1.set_ylabel('Δt (ms)', color='red')
		ax2_1.tick_params(axis='y', labelcolor='red')
		axs[1].legend(loc='upper left')
		ax2_1.legend(loc='upper right')
		axs[1].grid(True)

		# Plot onSurface for each sensor
		for sensor_id in df['sensor_id'].unique():
			sensor_df = df[df['sensor_id'] == sensor_id]
			axs[2].plot(sensor_df['time_sec'], sensor_df['onSurface'], label=f'Sensor {sensor_id}')
		ax2_2.plot(unique_sens_times, dt_sens, label='Δt', color='grey', alpha=0.5)
		axs[2].set_ylabel('On Surface')
		ax2_2.set_ylabel('Δt (ms)', color='red')
		ax2_2.tick_params(axis='y', labelcolor='red')
		axs[2].set_xlabel('Time (ms)')
		axs[2].legend(loc='upper left')
		ax2_2.legend(loc='upper right')
		axs[2].grid(True)
		
		# Plot sq (surface quality) for each sensor
		for sensor_id in df['sensor_id'].unique():
			sensor_df = df[df['sensor_id'] == sensor_id]
			axs[3].plot(sensor_df['time_sec'], sensor_df['sq'], label=f'Sensor {sensor_id}')
		ax2_2.plot(unique_sens_times, dt_sens, label='Δt', color='grey', alpha=0.5)
		axs[4].set_ylabel('Surface Quality')
		ax2_2.set_ylabel('Δt (ms)', color='red')
		ax2_2.tick_params(axis='y', labelcolor='red')
		axs[4].set_xlabel('Time (ms)')
		axs[4].legend(loc='upper left')
		ax2_2.legend(loc='upper right')
		axs[4].grid(True)

		# Plot angular velocities
		labels = [
			'[0-2] x', '[1-2] x', '[0-3] x', '[1-3] x',  # x velocity comparisons
			'[0-1] y', '[1-2] y', '[0-3] y', '[2-3] y'   # y velocity comparisons
		]
		for i in range(8):
			# Get ang_vel_array for each timestamp
			ang_vels = [data['ang_vel_array'][i] for data in df_processed.to_dict('records')]
			axs[4].plot(df_processed['time']/1_000, ang_vels, label=labels[i], alpha=0.7)

		# Plot average angular velocity
		axs[4].plot(df_processed['time']/1_000, df_processed['ang_vel'], 
					'k--', label='Average', linewidth=2)
		axs[4].set_ylabel('Angular Velocity')
		axs[4].set_xlabel('Time (ms)')
		axs[4].legend(loc='upper left')
		axs[4].grid(True)

		plt.suptitle('Sensor Data Over Time')
		plt.tight_layout()
		plt.show()

	def plot_time(self):
		"""Plot time data."""
		# Get DataFrames
		df_sens = self.get_sensor_dataframe()
		df_aux = self.get_aux_dataframe()
		
		# Get unique timestamps for sensor data
		unique_sens_times = df_sens['time'].unique()
		
		plt.figure(figsize=(12, 8))

		# Calculate time differences using unique timestamps
		dt_sens_diff = pd.Series(unique_sens_times).diff()/1_000
		dt_sens = [data['dt']/1_000 for data in self.sensor_data]
		dt_aux = df_aux['time'].diff()/1_000
		
		# Plot the machine position
		plt.plot(unique_sens_times/1_000, dt_sens_diff, label='Sensor Diff Data')
		plt.plot(unique_sens_times[1:]/1_000, dt_sens[1:], label='Sensor dt Data', linestyle='--')
		plt.scatter(df_aux['time']/1_000, dt_aux, label='Auxiliary Data')

		mplcursors.cursor(hover=True)

		plt.title('Sensor and Auxiliary Data Time Differences')
		plt.xlabel('Time (ms)')
		plt.ylabel('Time Difference (ms)')
		plt.legend()
		plt.grid(True)
		plt.show()

# Example usage
if __name__ == "__main__":
	# filename = input("Enter file name to decode: ")
	# decoder = BinaryLogDecoder(filename)
	# decoder = BinaryLogDecoder("../logFiles/proto-v1/LOG017.bin")
	decoder = BinaryLogDecoder("../logFiles/stephen/LOG065.bin")
	decoder.decode_file()
	
	# Print summary information
	print(f"\nDesign Info: {decoder.design_info}")
	print(f"Total Points: {len(decoder.points)}")
	print(f"Total Sensor Data Points: {len(decoder.sensor_data)}")
	print(f"Total Aux Data Points: {len(decoder.aux_data)}")
	
	# Convert to pandas DataFrames for analysis
	design_df = decoder.get_design_dataframe()
	sensor_df = decoder.get_sensor_dataframe()
	aux_df = decoder.get_aux_dataframe()
	
	# Save to CSV for further analysis if needed
	# design_df.to_csv("design_data.csv", index=False)
	# sensor_df.to_csv("sensor_data.csv", index=False)
	# aux_df.to_csv("aux_data.csv", index=False)
	
	# Generate plots
	# decoder.plot_design()
	decoder.plot_trajectory()
	decoder.plot_sensor_data()
	decoder.plot_time()