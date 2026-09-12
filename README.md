# Acer Battery GUI

A native, lightweight GTK3 graphical interface for managing Acer battery settings (Health Mode and Calibration) via the [RKKDR](https://github.com/RiiK26/RKKDR) Linux kernel module.


## Features

- **State-Aware Toggles**: Automatically reads your current battery settings from hardware upon launch, ensuring you never misdiagnose your battery's state.
- **Health Mode**: Toggle the 80% charge limit to preserve battery lifespan.
- **Calibration Mode**: Toggle the battery calibration state.
- **Live Temperature**: Real-time polling of your battery's temperature directly from the hardware.

## Prerequisites

To compile and run this application, you must have the following installed on your system:
- **GTK3 Development Libraries** (`libgtk-3-dev` or `gtk3-devel` depending on your distro)
- **Make** and **GCC**
- **Polkit (`pkexec`)**: Used to safely prompt for root permissions without hardcoding passwords.

## Installation

1. **Clone the project**:
   ```bash
   git clone https://github.com/RiiK26/AcerBattery.git
   cd AcerBattery
   ```
2. **Run the installation script**:
   ```bash
   ./install.sh
   ```
   This script will automatically initialize the `RKKDR` submodule if needed, compile the GUI application, and install the `RKKDR` kernel driver via DKMS so it persists across kernel updates.

## Desktop Integration

You can launch the GUI directly from the provided launcher script:
```bash
./launcher.sh
```

**What the launcher does:**
1. Checks if the `RKKDR` kernel module is loaded in your system.
2. If it isn't, it will automatically find the source code from your local `RKKDR` submodule (or fall back to a developer workspace) to compile and load the driver on-the-fly.
3. Grants local root X11 access.
4. Uses `pkexec` to prompt you for your password visually, and then runs the GUI with the necessary permissions to write to sysfs.

To add this application to your desktop menu:
1. Copy the `.desktop` file (or create one using the template below) to `~/.local/share/applications/acerbattery.desktop`.
2. Run `update-desktop-database ~/.local/share/applications/`.

## Maintaining the Kernel Module Submodule

This project keeps a reference to the upstream `RKKDR` kernel module using a git submodule, but is intentionally designed to **not** clone the duplicate files to your hard drive, keeping your workspace clean.

If you ever need to update the submodule commit reference to match upstream changes, just run the provided script:
```bash
./update_submodule.sh
```
This script will fetch the latest commits, update your submodule reference, commit the change, and immediately clean up the copied files again.

## License
This project is licensed under the GPL-3.0 [LICENSE](License)