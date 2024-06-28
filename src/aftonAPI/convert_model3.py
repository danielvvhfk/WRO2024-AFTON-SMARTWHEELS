import tvm
from tvm import relay
import onnx

# Load the ONNX model
onnx_model = onnx.load("steering_model.onnx")

# Define the shape dictionary with the correct input name and shape
shape_dict = {"conv2d_input": (32, 50, 50, 3)}  # Input shape (batch size, height, width, channels)

# Convert ONNX model to Relay IR
mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)

# Target configuration
target = tvm.target.cuda()  # You can change the target based on your hardware
target_host = "llvm"

# Build the module
with tvm.transform.PassContext(opt_level=3):
    lib = relay.build(mod, target=target, target_host=target_host, params=params)

# Save the compiled module
lib.export_library("steering_model.tar")
