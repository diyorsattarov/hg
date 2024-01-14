<template>
  <div class="about">
    <h1>This is an about page</h1>
    <div v-if="loading">Loading...</div>
    <div v-else>
      <h2>Results12312 from /get/1:</h2>
      <table>
        <thead>
          <tr>
            <th v-for="(value, key) in result[0]" :key="key">{{ key }}</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="row in result" :key="row['0']">
            <td v-for="(value, key) in row" :key="key">
              <!-- Check if the value is a date to remove the year -->
              <span v-if="key === '1' || key === '2'">{{ formatDate(value) }}</span>
              <span v-else>{{ value }}</span>
            </td>
          </tr>
        </tbody>
      </table>

      <div class="request-form">
        <h2>Make a Request</h2>
        <label for="jsonInput">JSON Input:</label>
        <textarea v-model="jsonInput" id="jsonInput"></textarea>

        <label for="httpMethod">HTTP Method:</label>
        <select v-model="selectedMethod" id="httpMethod">
          <option value="POST">POST</option>
          <option value="PUT">PUT</option>
          <option value="DELETE">DELETE</option>
        </select>

        <button @click="sendRequest">Send Request</button>
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
      updatedResult: null,
    };
  },
  mounted() {
    this.fetchData();
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
    formatDate(dateString) {
      const date = new Date(dateString);
      const options = { hour: 'numeric', minute: 'numeric', hour12: false };
      return date.toLocaleDateString('en-US', options);
    },
    sendRequest() {
      let endpoint = '/insert/1';

      if (this.selectedMethod === 'PUT') {
        endpoint = '/update/1';
      } else if (this.selectedMethod === 'DELETE') {
        endpoint = '/delete/1';
      }

      const fullEndpoint = `http://localhost:3000${endpoint}`;

      const requestOptions = {
        method: this.selectedMethod,
        headers: {
          'Content-Type': 'application/json',
        },
        body: this.jsonInput,
      };

      fetch(fullEndpoint, requestOptions)
        .then((response) => response.json())
        .then((data) => {
          this.updatedResult = data;
          // Display the response beneath the form
          alert(`Response: ${JSON.stringify(data)}`);
        })
        .catch((error) => {
          console.error('Error sending request:', error);
          this.updatedResult = 'Error sending request';
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
}
</style>
