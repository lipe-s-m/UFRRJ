package classesp2;

public class Cachorro 
{
	// Atributos
	String cor;
	private String nome = "Luna";
	private int idade = 2;
	
	
	//get-set idade
	public int getIdade()
	{
		return idade;
	}
	
	public void setIdade(int idadeNovo)
	{
		idade = idadeNovo;
	}
	
	
	//get-set nome
	public String getNome()
	{
		return nome;
	}
	
	public void setNome(String nomeNovo)
	{
		nome = nomeNovo;
	}
	
	
	// Métodos
	public void late()
	{
		System.out.printf("%s está latindo: auauuu!", nome);
	}
	
	
	

}
