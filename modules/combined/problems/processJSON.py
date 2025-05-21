# %%
## Imports
# %cd /Users/dhulls/projects/moose/modules/combined/examples/stochastic/STM_training/BayesianUQ
import json
import numpy as np
from matplotlib import pyplot as plt
import math

# %%
# Process data

file = '/Users/otchc/projects/moose/modules/combined/problems/al1.json'
num_iter = 50
parallel_props = 5
dim = 2

f = open(file,)
data = json.load(f)
inputs = np.zeros((num_iter,parallel_props,dim))
obj_values = np.zeros((num_iter,parallel_props))
for ii in np.arange(1,num_iter+1,1):
    inputs[ii-1,:,:] = np.array(data["time_steps"][ii]['conditional']['inputs'])
    obj_values[ii-1, :] = np.array(data["time_steps"][ii]['conditional']['outputs_required'])

# plt.plot(np.maximum.accumulate(obj_values))
# plt.xlabel('Iteration')
# plt.ylabel('log inverse error')

index_max = np.argmax(obj_values[:,0])

print('Optimized input values: '+str(inputs[index_max,0,:]))
print('Optimized objective (log inv error): '+str(obj_values[index_max,0]))

plt.plot(inputs[:,0,0], marker='o', linestyle='-')
plt.axhline(y=1.0, color='k', linewidth=2)
plt.xlabel('Iteration')
plt.ylabel('Velocity BC 1')

plt.plot(inputs[:,0,1], marker='o', linestyle='-')
plt.axhline(y=2.5, color='k', linewidth=2)
plt.xlabel('Iteration')
plt.ylabel('Velocity BC 2')
# %%
