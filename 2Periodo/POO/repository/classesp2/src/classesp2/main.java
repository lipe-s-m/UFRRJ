package classesp2;

public class main {
	//main
	public static void main(String[] args) 
	{
		int idade = 0;
		String nome;
		
		//Criando Objeto
		Cachorro cachorro1 = new Cachorro();
		nome = cachorro1.getNome();
		
		cachorro1.late();
		
		//mostrando a atributos originais
		idade = cachorro1.getIdade();
		System.out.printf("\n%s tem %d anos!", nome, idade);
		
		//mostrando atributos modificados
		cachorro1.setIdade(4);
		cachorro1.setNome("Darwin");
		nome = cachorro1.getNome();
		idade = cachorro1.getIdade();
		
		System.out.printf("\n\nAgora %s tem %d anos!", nome, idade);
	}
}
