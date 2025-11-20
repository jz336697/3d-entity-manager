#!/usr/bin/env python3
"""
Create simple billboard images for LOD optimization
Requires: pip install pillow
"""

try:
    from PIL import Image, ImageDraw
except ImportError:
    print("Error: PIL (Pillow) is not installed.")
    print("Install with: pip install pillow")
    exit(1)

def create_ship_icon(size=256, output_path="ship_icon.png"):
    """Create a blue triangle ship icon"""
    # Create transparent image
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Draw blue triangle (ship silhouette)
    points = [
        (size // 2, size // 5),           # Top point
        (size // 5, size * 4 // 5),       # Bottom left
        (size * 4 // 5, size * 4 // 5),   # Bottom right
    ]
    draw.polygon(points, fill=(68, 136, 255, 255))  # Blue #4488FF
    
    # Add outline for better visibility
    draw.polygon(points, outline=(40, 90, 180, 255), width=2)
    
    img.save(output_path)
    print(f"Created: {output_path}")

def create_missile_icon(size=256, output_path="missile_icon.png"):
    """Create a red arrow missile icon"""
    # Create transparent image
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Draw red arrow (missile silhouette)
    center_x = size // 2
    
    # Arrow shaft
    shaft_top = size // 8
    shaft_bottom = size * 3 // 4
    shaft_width = size // 6
    
    shaft_points = [
        (center_x - shaft_width // 2, shaft_top),
        (center_x + shaft_width // 2, shaft_top),
        (center_x + shaft_width // 2, shaft_bottom),
        (center_x - shaft_width // 2, shaft_bottom),
    ]
    draw.polygon(shaft_points, fill=(255, 68, 68, 255))  # Red #FF4444
    
    # Arrow head
    head_points = [
        (center_x, size // 16),                    # Top point
        (center_x - shaft_width, shaft_top),       # Left
        (center_x + shaft_width, shaft_top),       # Right
    ]
    draw.polygon(head_points, fill=(255, 68, 68, 255))  # Red #FF4444
    
    # Arrow tail fins
    tail_points = [
        (center_x, size * 7 // 8),                 # Bottom point
        (center_x - shaft_width, shaft_bottom),    # Left
        (center_x + shaft_width, shaft_bottom),    # Right
    ]
    draw.polygon(tail_points, fill=(255, 68, 68, 255))  # Red #FF4444
    
    # Add outline for better visibility
    draw.polygon(shaft_points, outline=(180, 40, 40, 255), width=2)
    draw.polygon(head_points, outline=(180, 40, 40, 255), width=2)
    draw.polygon(tail_points, outline=(180, 40, 40, 255), width=2)
    
    img.save(output_path)
    print(f"Created: {output_path}")

def main():
    """Create both billboard images"""
    print("Creating billboard images for LOD optimization...")
    print("Size: 256x256 pixels, Format: PNG with transparency")
    print()
    
    create_ship_icon(256, "ship_icon.png")
    create_missile_icon(256, "missile_icon.png")
    
    print()
    print("Billboard images created successfully!")
    print("Place these images in: resource/images/")

if __name__ == "__main__":
    main()
