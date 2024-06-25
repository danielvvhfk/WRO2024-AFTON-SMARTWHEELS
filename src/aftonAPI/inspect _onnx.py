import onnx

# Load the ONNX model
onnx_model = onnx.load("steering_model.onnx")

# Print a human-readable representation of the graph
print(onnx.helper.printable_graph(onnx_model.graph))