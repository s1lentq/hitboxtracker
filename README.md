# Description
Dev-tool that demonstrates on client-side true position of the hitboxes calculated by server

# Installation
* Install `hitboxtracker_mm.dll` on your HLDS as metamod plugin and put `delta.lst` to gamedir directory `cstrike/delta.lst`
* Put `hitboxtracker.dll` and `cs.exe` to working directory of the game CS 1.6 client
* Run `cs.exe`

# Requirements
* For `Client`: build 4554 and later
* For `HLDS`: Metamod 1.20 and later

# Configuring
<table>
	<tbody>
		<tr>
			<th>r_drawentities</th>
			<th align="left">Description</th>
		</tr>
		<tr>
			<td>0</td>
			<td align="left">No entities</td>
		</tr>
		<tr>
			<td>1</td>
			<td align="left">Default and draws entities normally</td>
		</tr>
		<tr>
			<td>2</td>
			<td align="left">Entities draws as skeletons</td>
		</tr>
		<tr>
			<td>3</td>
			<td align="left">Entities draws hitboxes</td>
		</tr>
		<tr>
			<td>4</td>
			<td align="left">Entities draws with translucent hitboxes and the model beneath the hitboxes</td>
		</tr>
		<tr>
			<td>5</td>
			<td align="left">Individual box for player and weapon</td>
		</tr>
		<tr>
			<td>6</td>
			<td align="left">Same as 4 but draws true position of the hitboxes calculated by server :new:</td>
		</tr>
	</tbody>
</table>

# Preview
![r_drawentities-preview](https://user-images.githubusercontent.com/5860435/34445881-52d2096a-ed09-11e7-90ad-78cfb38f3f40.gif)
