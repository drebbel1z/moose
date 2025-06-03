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
num_iter = 17
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
# plt.xlabel('Iteration')
# plt.ylabel('Velocity BC 1')

plt.plot(inputs[:,0,1], marker='o', linestyle='-')
plt.axhline(y=2.5, color='k', linewidth=2)
plt.xlabel('Iteration')
plt.ylabel('Velocity BC')




# %%
maxima=[]
locations_of_maxima=[]

for i in range(num_iter):
    index_of_max=np.argmax(obj_values[:i+1,:],axis=0)
    maxima.append(np.expand_dims(obj_values[index_of_max,np.arange(parallel_props)],0))
    locations_of_maxima.append(np.expand_dims(inputs[index_of_max,np.arange(parallel_props),:],0))

maxima=np.concatenate(maxima,axis=0)
locations_of_maxima =np.concatenate(locations_of_maxima,axis=0)
for i in range(parallel_props):
    plt.plot(maxima[:,i],"-x",label=f"prop{i}")
plt.xticks(np.arange(num_iter,step=5))
plt.legend()
plt.xlabel("iteration number")
plt.ylabel(r"$log\left(\frac{1}{MSE}\right)$")
plt.show()

for i in range(parallel_props):
    trajectory=locations_of_maxima[:,i,:]
    plt.plot(trajectory[:,0],"k")
    plt.plot(trajectory[:,1],"k")
plt.axhline(y=1.0, color='r', linewidth=2, label=r"optimal $u_{top_x}$")
plt.axhline(y=2.5, color='b', linewidth=2, label=r"optimal $u_{right_y}$")

plt.axhline(y=-1.0, color='r', linewidth=2,)
plt.axhline(y=-2.5, color='b', linewidth=2,)

plt.xticks(np.arange(num_iter,step=5))
plt.legend()
plt.xlabel("iteration number")
plt.ylabel("Velocity at BC")
    


# %%
MSE=1/np.exp(maxima)

for i in range(parallel_props):
    plt.plot(MSE[:,i],"-x",label=f"prop{i}")
plt.xticks(np.arange(num_iter,step=2))
plt.legend()
plt.xlabel("iteration number")
plt.ylabel(r"$MSE$")
plt.yscale("log")
plt.show()
# %%

# %%
