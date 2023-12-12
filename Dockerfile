FROM node:20.10.0-alpine

# Step 1: Setup workdir
WORKDIR /sirius-api

# Step 2: Copy package.json and install dependencies
COPY package*.json ./
RUN npm install --production 

# Step 3: Copy the source code into the sirius-api folder
COPY . .

# Step 4: Expose container port
EXPOSE 3000

# Step 5: Setup environment variables
ENV PORT=3000 ADDRESS=0.0.0.0

# Step 6: Run the application
CMD ["npm", "start"]

# Step 7: Changing the default user
USER node 



