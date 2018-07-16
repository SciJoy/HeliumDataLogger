/**
 * Background Cloud Function to be triggered by PubSub.
 *
 * @param {object} event The Cloud Functions event.
 * @param {function} callback The callback function.
 */
exports.subscribe = function (event, callback) {
    const BigQuery = require('@google-cloud/bigquery');
    const projectId = "YourProject"; //Enter your project ID here
    const datasetId = "YourDatabase"; //Enter your BigQuery dataset name here
    const tableId = "YourTable"; //Enter your BigQuery table name here -- make sure it is setup correctly
    const PubSubMessage = event.data;
    // Incoming data is in JSON format
    const incomingData = PubSubMessage.data ? Buffer.from(PubSubMessage.data, 'base64').toString() : "{'E':'1','S':'na','T':'00000000000','C':'0000','M':'0000','X':'0000','A':'0000'}";
    const jsonData = JSON.parse(incomingData);
    
    
    var rows = [jsonData];
    
    console.log(`Incoming data: ${rows}`);
    
    // Instantiates a client
    const bigquery = BigQuery({
                              projectId: projectId
                              });
    
    // Inserts data into a table
    bigquery
    .dataset(datasetId)
    .table(tableId)
    .insert(rows)
    .then((insertErrors) => {
          console.log('Inserted:');
          rows.forEach((row) => console.log(row));
          
          if (insertErrors && insertErrors.length > 0) {
          console.log('Insert errors:');
          insertErrors.forEach((err) => console.error(err));
          }
          })
    .catch((err) => {
           console.error('ERROR:', err);
           });
    // [END bigquery_insert_stream]
    
    callback();
};
