<template>
  <div class="about">
    <h1>This is an about page</h1>
    <div v-if="loading">Loading...</div>
    <div v-else>
      <h2>Results from /get/1:</h2>
      <template v-if="result && result.data && result.data.length > 0">
        <table>
          <tbody>
            <tr v-for="row in sortedResult" :key="row.task_id">
              <td>{{ row.task_id }}</td>
              <td>{{ row.task }}</td>
              <td>{{ row.day_id }}</td>
              <td>{{ formatTime(row.start_datetime) }}</td>
              <td>{{ formatTime(row.end_datetime) }}</td>
              <td>
                <button @click="editRow(row)">Edit</button>
              </td>
            </tr>
          </tbody>
        </table>
      </template>
      <template v-else>
        No elements found.
      </template>


      <!-- Display the response beneath the form -->
      <div v-if="updatedResult">
        <h2>Response:</h2>
        <pre>{{ JSON.stringify(updatedResult, null, 2) }}</pre>
      </div>

      <!-- Insert Form -->
      <div class="request-form" style="text-align: center;">
        <h2>Insert Data</h2>

        <!-- Input field for "task" column -->
        <div>
          <label for="taskInput">Task:</label>
          <input v-model="taskInput" id="taskInput" />
        </div>

        <!-- Input field for "start_datetime" column -->
        <div>
          <label for="startDatetimeInput">Start Datetime:</label>
          <input v-model="startDatetimeInput" id="startDatetimeInput" />
        </div>

        <!-- Input field for "end_datetime" column -->
        <div>
          <label for="endDatetimeInput">End Datetime:</label>
          <input v-model="endDatetimeInput" id="endDatetimeInput" />
        </div>

        <!-- Input field for "day_id" column -->
        <div>
          <label for="dayIdInput">Day ID:</label>
          <input v-model="dayIdInput" id="dayIdInput" />
        </div>

        <div>
          <label for="taskIdInput">Task ID:</label>
          <input v-model="taskIdInput" id="taskIdInput" />
        </div>

        <!-- Dropdown for HTTP Method -->
        <div>
          <label for="httpMethodInsert">HTTP Method:</label>
          <select v-model="selectedMethodInsert" id="httpMethodInsert">
            <option value="POST">POST</option>
          </select>
        </div>

        <button @click="insertData">Insert Data</button>
      </div>

      <!-- Update Form -->
      <div class="request-form" style="text-align: center;">
        <h2>Update Data</h2>

        <!-- Input field for "task" column -->
        <div>
          <label for="taskUpdateInput">Task:</label>
          <input v-model="taskUpdateInput" id="taskUpdateInput" />
        </div>

        <!-- Input field for "start_datetime" column -->
        <div>
          <label for="startDatetimeUpdateInput">Start Datetime:</label>
          <input v-model="startDatetimeUpdateInput" id="startDatetimeUpdateInput" />
        </div>

        <!-- Input field for "end_datetime" column -->
        <div>
          <label for="endDatetimeUpdateInput">End Datetime:</label>
          <input v-model="endDatetimeUpdateInput" id="endDatetimeUpdateInput" />
        </div>

        <!-- Input field for "day_id" column -->
        <div>
          <label for="dayIdUpdateInput">Day ID:</label>
          <input v-model="dayIdUpdateInput" id="dayIdUpdateInput" />
        </div>

        <div>
          <label for="taskIdUpdateInput">Task ID:</label>
          <input v-model="taskIdUpdateInput" id="taskIdUpdateInput" />
        </div>

        <!-- Dropdown for HTTP Method -->
        <div>
          <label for="httpMethodUpdate">HTTP Method:</label>
          <select v-model="selectedMethodUpdate" id="httpMethodUpdate">
            <option value="PUT">PUT</option>
          </select>
        </div>

        <button @click="updateData">Update Data</button>
      </div>

      <!-- Delete Form -->
      <div class="request-form" style="text-align: center;">
        <h2>Delete Data</h2>

        <div>
          <label for="taskIdDeleteInput">Task ID:</label>
          <input v-model="taskIdDeleteInput" id="taskIdDeleteInput" />
        </div>

        <!-- Dropdown for HTTP Method -->
        <div>
          <label for="httpMethodDelete">HTTP Method:</label>
          <select v-model="selectedMethodDelete" id="httpMethodDelete">
            <option value="DELETE">DELETE</option>
          </select>
        </div>

        <button @click="deleteData">Delete Data</button>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  data() {
    return {
      loading: true,
      result: null,
      jsonInput: '',
      selectedMethod: 'POST',
      selectedMethodInsert: 'POST',
      taskInput: '',
      taskIdInput: '',
      startDatetimeInput: '',
      endDatetimeInput: '',
      dayIdInput: '',
      updatedResult: null,
    };
  },
  mounted() {
    this.fetchData();
  },
  computed: {
    sortedResult() {
      return this.result.data.slice().sort((a, b) => {
        // Convert the date strings to Date objects for proper comparison
        const dateA = new Date(a.start_datetime);
        const dateB = new Date(b.start_datetime);

        // Sort in descending order (most recent to latest)
        return dateB - dateA;
      });
    },
  },
  methods: {
    fetchData() {
      fetch('http://localhost:3000/get/1')
        .then((response) => response.json())
        .then((data) => {
          this.loading = false;
          this.result = data;
        })
        .catch((error) => {
          console.error('Error fetching data:', error);
          this.loading = false;
          this.result = 'Error fetching data';
        });
    },
    formatTime(dateTimeString) {
      const date = new Date(dateTimeString);
      const options = { hour: 'numeric', minute: 'numeric', hour12: false };
      return date.toLocaleTimeString('en-US', options);
    },
    formatDate(dateString) {
      const date = new Date(dateString);
      const options = { hour: 'numeric', minute: 'numeric', hour12: false };
      return date.toLocaleDateString('en-US', options);
    },
    insertData() {
      const fullEndpoint = 'http://localhost:3000/insert/1';

      const requestData = {
        columns: ["task", "start_datetime", "end_datetime", "day_id"],
        values: [this.taskInput, this.startDatetimeInput, this.endDatetimeInput, this.dayIdInput],
      };

      const requestOptions = {
        method: this.selectedMethodInsert,
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(requestData),
      };

      fetch(fullEndpoint, requestOptions)
        .then((response) => response.json())
        .then((data) => {
          this.updatedResult = data;
          // Update the table data after a successful request
          this.fetchData();
        })
        .catch((error) => {
          console.error('Error inserting data:', error);
          this.updatedResult = 'Error inserting data';
        });
      1
    },
    updateData() {
      const fullEndpoint = `http://localhost:3000/update/1`;

      // Get the column names from the fetched data
      const columnNames = Object.keys(this.result[0]);

      // Build the requestData object
      const requestData = {
        columns: [],
        values: [],
        where: `task_id='${this.taskIdUpdateInput}' AND day_id='${this.dayIdUpdateInput}'`, // Assuming 'task_id' is the unique identifier
      };

      // Add fields to requestData.columns and requestData.values only if they have a value
      columnNames.forEach((columnName) => {
        const inputValue = this[columnName + 'UpdateInput'];
        if (inputValue !== undefined && inputValue !== null) {
          requestData.columns.push(columnName);
          requestData.values.push(inputValue);
        }
      });

      const requestOptions = {
        method: this.selectedMethodUpdate,
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(requestData),
      };

      fetch(fullEndpoint, requestOptions)
        .then((response) => response.json())
        .then((data) => {
          this.updatedResult = data;
          // Update the table data after a successful request
          this.fetchData();
        })
        .catch((error) => {
          console.error('Error updating data:', error);
          this.updatedResult = 'Error updating data';
        });
    },

    deleteData() {
      const fullEndpoint = `http://localhost:3000/delete/1`;

      const requestData = {
        where: `task_id='${this.taskIdDeleteInput}'`, // Assuming 'task_id' is the unique identifier
      };

      const requestOptions = {
        method: this.selectedMethodDelete,
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(requestData),
      };

      fetch(fullEndpoint, requestOptions)
        .then((response) => response.json())
        .then((data) => {
          this.updatedResult = data;
          // Update the table data after a successful request
          this.fetchData();
        })
        .catch((error) => {
          console.error('Error deleting data:', error);
          this.updatedResult = 'Error deleting data';
        });
    },
  },
};
</script>

<style>
@media (min-width: 1024px) {
  .about {
    min-height: 100vh;
    display: flex;
    align-items: center;
    flex-direction: column;
  }

  table {
    width: 80%;
    margin-top: 20px;
    border-collapse: collapse;
  }

  th,
  td {
    border: 1px solid #dddddd;
    text-align: left;
    padding: 8px;
  }

  th {
    background-color: #f2f2f2;
  }

  .request-form {
    margin-top: 20px;
  }
}
</style>
